
#include <cinttypes>

#include <tlCore/FontSystem.h>
#include <tlCore/Mesh.h>

#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Render.h>
#include <tlGL/Shader.h>
#include <tlGL/Util.h>


#include <tlGlad/gl.h>

// mrViewer includes
#include <mrvCore/mrvUtil.h>
#include <mrvCore/mrvSequence.h>
#include <mrvCore/mrvColorSpaces.h>

#include <mrvFl/mrvIO.h>
#include "mrvFl/mrvToolsCallbacks.h"
#include <mrvFl/mrvTimelinePlayer.h>
#include <mrViewer.h>

#include <mrvGL/mrvGLShape.h> // remove
#include <mrvGL/mrvTimelineViewport.h>
#include <mrvGL/mrvTimelineViewportPrivate.h>
#include <mrvGL/mrvGLViewport.h>

#include <glm/gtc/matrix_transform.hpp>

#include "mrvGLErrors.h"
#include "mrvGLUtil.h"

// For main fltk event loop
#include <FL/Fl.H>

//! Define a variable, "gl", that references the private implementation.
#define TLRENDER_GL()                           \
    auto& gl = *_gl

#define USE_ONE_PIXEL_LINES 1

namespace {
    const char* kModule = "glview";
}

namespace mrv
{
    using namespace tl;

    struct GLViewport::GLPrivate
    {
        std::weak_ptr<system::Context> context;

        // GL variables
        //! OpenGL Offscreen buffer
        std::shared_ptr<tl::gl::OffscreenBuffer> buffer = nullptr;
        std::shared_ptr<tl::gl::Render> render = nullptr;
        std::shared_ptr<tl::gl::Shader> shader    = nullptr;
        int index = 0;
        int nextIndex = 1;
        GLuint pboIds[2];
        std::shared_ptr<gl::VBO> vbo;
        std::shared_ptr<gl::VAO> vao;

        GLfloat*                 image = nullptr;
    };


    GLViewport::GLViewport( int X, int Y, int W, int H, const char* L ) :
        TimelineViewport( X, Y, W, H, L ),
        _gl( new GLPrivate )
    {
        mode( FL_RGB | FL_DOUBLE | FL_ALPHA | FL_STENCIL | FL_OPENGL3 );
    }


    GLViewport::GLViewport( int W, int H, const char* L ) :
        TimelineViewport( W, H, L ),
        _gl( new GLPrivate )
    {
        mode( FL_RGB | FL_DOUBLE | FL_ALPHA | FL_STENCIL | FL_OPENGL3 );
    }


    GLViewport::~GLViewport()
    {
        TLRENDER_GL();

        glDeleteBuffers(2, gl.pboIds);
    }

    void GLViewport::setContext(
        const std::weak_ptr<system::Context>& context )
    {
        _gl->context = context;
    }

    void GLViewport::initializeGL()
    {
        TLRENDER_P();
        TLRENDER_GL();
        try
        {
            tl::gl::initGLAD();


            if ( !gl.render )
            {
                if (auto context = gl.context.lock())
                {
                    gl.render = gl::Render::create(context);
                    p.fontSystem = imaging::FontSystem::create(context);
                    glGenBuffers( 2, gl.pboIds );
                }
            }

            if ( !gl.shader )
            {

                const std::string vertexSource =
                    "#version 410\n"
                    "\n"
                    "in vec3 vPos;\n"
                    "in vec2 vTexture;\n"
                    "out vec2 fTexture;\n"
                    "\n"
                    "uniform struct Transform\n"
                    "{\n"
                    "    mat4 mvp;\n"
                    "} transform;\n"
                    "\n"
                    "void main()\n"
                    "{\n"
                    "    gl_Position = transform.mvp * vec4(vPos, 1.0);\n"
                    "    fTexture = vTexture;\n"
                    "}\n";
                const std::string fragmentSource =
                    "#version 410\n"
                    "\n"
                    "in vec2 fTexture;\n"
                    "out vec4 fColor;\n"
                    "\n"
                    "uniform sampler2D textureSampler;\n"
                    "\n"
                    "void main()\n"
                    "{\n"
                    "    fColor = texture(textureSampler, fTexture);\n"
                    "}\n";
                gl.shader = gl::Shader::create(vertexSource, fragmentSource);
            }
        }
        catch (const std::exception& e)
        {
            if (auto context = gl.context.lock())
            {
                context->log(
                    "mrv::GLViewport",
                    e.what(),
                    log::Type::Error);
            }
        }
    }



    void GLViewport::draw()
    {
        TLRENDER_P();
        TLRENDER_GL();

        if ( !valid() )
        {
            initializeGL();
            valid(1);
        }

        const auto renderSize = _getRenderSize();
        try
        {
            if (renderSize.isValid())
            {
                gl::OffscreenBufferOptions offscreenBufferOptions;
                offscreenBufferOptions.colorType = imaging::PixelType::RGBA_F32;
                if (!p.displayOptions.empty())
                {
                    offscreenBufferOptions.colorFilters = p.displayOptions[0].imageFilters;
                }
                offscreenBufferOptions.depth = gl::OffscreenDepth::_24;
                offscreenBufferOptions.stencil = gl::OffscreenStencil::_8;
                if (gl::doCreate(gl.buffer, renderSize, offscreenBufferOptions))
                {
                    gl.buffer = gl::OffscreenBuffer::create(renderSize, offscreenBufferOptions);
                    unsigned dataSize = renderSize.w * renderSize.h  * 4
                                        * sizeof(GLfloat);
                    glBindBuffer(GL_PIXEL_PACK_BUFFER, gl.pboIds[0]);
                    glBufferData(GL_PIXEL_PACK_BUFFER, dataSize, 0,
                                 GL_STREAM_READ);
                    glBindBuffer(GL_PIXEL_PACK_BUFFER, gl.pboIds[1]);
                    glBufferData(GL_PIXEL_PACK_BUFFER, dataSize, 0,
                                 GL_STREAM_READ);
                    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
                    CHECK_GL;
                }
            }
            else
            {
                gl.buffer.reset();
            }

            if (gl.buffer)
            {
                gl::OffscreenBufferBinding binding(gl.buffer);
                gl.render->setColorConfig(p.colorConfigOptions);
                gl.render->setLUT(p.lutOptions);
                CHECK_GL;
                gl.render->begin(renderSize);
                CHECK_GL;
                gl.render->drawVideo(
                    p.videoData,
                    timeline::tiles(p.compareOptions.mode,
                                    _getTimelineSizes()),
                    p.imageOptions,
                    p.displayOptions,
                    p.compareOptions);
                if (p.masking > 0.0001F ) _drawCropMask( renderSize );
                gl.render->end();
            }
        }
        catch (const std::exception& e)
        {
            if (auto context = gl.context.lock())
            {
                context->log(
                    "mrv::GLViewport",
                    e.what(),
                    log::Type::Error);
            }
        }

        const auto viewportSize = _getViewportSize();
        glViewport(
            0,
            0,
            GLsizei(viewportSize.w),
            GLsizei(viewportSize.h));

        float r, g, b, a = 0.0f;
        if ( !p.presentation )
        {
            uint8_t ur, ug, ub;
            Fl::get_color( p.ui->uiPrefs->uiPrefsViewBG->color(), ur, ug, ub );
            r = ur / 255.0f;
            g = ur / 255.0f;
            b = ur / 255.0f;
        }
        else
        {
            r = g = b = a = 0.0f;
        }
        glClearColor( r, g, b, a );
        glClear(GL_COLOR_BUFFER_BIT);

        if (gl.buffer)
        {
            gl.shader->bind();
            glm::mat4x4 vm(1.F);
            vm = glm::translate(vm, glm::vec3(p.viewPos.x, p.viewPos.y, 0.F));
            vm = glm::scale(vm, glm::vec3(p.viewZoom, p.viewZoom, 1.F));
            const glm::mat4x4 pm = glm::ortho(
                0.F,
                static_cast<float>(viewportSize.w),
                0.F,
                static_cast<float>(viewportSize.h),
                -1.F,
                1.F);
            glm::mat4x4 vpm = pm * vm;
            auto mvp = math::Matrix4x4f(
                vpm[0][0], vpm[0][1], vpm[0][2], vpm[0][3],
                vpm[1][0], vpm[1][1], vpm[1][2], vpm[1][3],
                vpm[2][0], vpm[2][1], vpm[2][2], vpm[2][3],
                vpm[3][0], vpm[3][1], vpm[3][2], vpm[3][3] );

            gl.shader->setUniform("transform.mvp", mvp);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, gl.buffer->getColorID());

            geom::TriangleMesh3 mesh;
            mesh.v.push_back(math::Vector3f(0.F, 0.F, 0.F));
            mesh.t.push_back(math::Vector2f(0.F, 0.F));
            mesh.v.push_back(math::Vector3f(renderSize.w, 0.F, 0.F));
            mesh.t.push_back(math::Vector2f(1.F, 0.F));
            mesh.v.push_back(math::Vector3f(renderSize.w, renderSize.h, 0.F));
            mesh.t.push_back(math::Vector2f(1.F, 1.F));
            mesh.v.push_back(math::Vector3f(0.F, renderSize.h, 0.F));
            mesh.t.push_back(math::Vector2f(0.F, 1.F));
            mesh.triangles.push_back(geom::Triangle3({
                        geom::Vertex3({ 1, 1, 0 }),
                        geom::Vertex3({ 2, 2, 0 }),
                        geom::Vertex3({ 3, 3, 0 })
                    }));
            mesh.triangles.push_back(geom::Triangle3({
                        geom::Vertex3({ 3, 3, 0 }),
                        geom::Vertex3({ 4, 4, 0 }),
                        geom::Vertex3({ 1, 1, 0 })
                    }));

            auto vboData = convert(
                mesh,
                gl::VBOType::Pos3_F32_UV_U16,
                math::SizeTRange(0, mesh.triangles.size() - 1));
            if (!gl.vbo)
            {
                gl.vbo = gl::VBO::create(mesh.triangles.size() * 3, gl::VBOType::Pos3_F32_UV_U16);
            }
            if (gl.vbo)
            {
                gl.vbo->copy(vboData);
            }

            if (!gl.vao && gl.vbo)
            {
                gl.vao = gl::VAO::create(gl::VBOType::Pos3_F32_UV_U16, gl.vbo->getID());
            }
            if (gl.vao && gl.vbo)
            {
                gl.vao->bind();
                gl.vao->draw(GL_TRIANGLES, 0, gl.vbo->getSize());

                math::BBox2i selection = p.colorAreaInfo.box = p.selection;
                if ( selection.min != selection.max )
                {
                    // Check min < max
                    if ( selection.min.x > selection.max.x )
                    {
                        float tmp = selection.max.x;
                        selection.max.x = selection.min.x;
                        selection.min.x = tmp;
                    }
                    if ( selection.min.y > selection.max.y )
                    {
                        float tmp = selection.max.y;
                        selection.max.y = selection.min.y;
                        selection.min.y = tmp;
                    }
                    // Copy it again in cae it changed
                    p.colorAreaInfo.box = selection;
                    calculateColorAreaInfo( selection, p.colorAreaInfo );
                }
                if ( colorAreaTool ) colorAreaTool->update( p.colorAreaInfo );
                updatePixelBar();

                if ( gl.image )
                {
                    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
                    gl.image = nullptr;
                }

                // back to conventional pixel operation
                glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);


                Fl_Color c = p.ui->uiPrefs->uiPrefsViewSelection->color();
                uint8_t r, g, b;
                Fl::get_color( c, r, g, b );

                const imaging::Color4f color(r / 255.F, g / 255.F,
                                             b / 255.F);
                    
                if ( p.selection.min != p.selection.max )
                {
#if USE_ONE_PIXEL_LINES
                    gl.render->setMatrix(mvp);
                    gl.render->drawRectOutline( p.selection, color );
#else
                    drawRectOutline( gl.render, p.selection, color, 2.F, mvp );
#endif
                }
                
                if ( p.showAnnotations ) _drawAnnotations(mvp);
                
                if ( p.actionMode != ActionMode::kScrub &&
                     p.actionMode != ActionMode::kText &&
                     p.actionMode != ActionMode::kSelection )
                {
                    float pen_size = 10.F;
                    switch ( p.actionMode )
                    {
                    case ActionMode::kRectangle:
                    case ActionMode::kCircle:
                    case ActionMode::kArrow:
                        pen_size = 2;
                        break;
                    default:
                        break;
                    }

                    drawCursor( gl.render, _getRaster(), pen_size, 2.0F,
                                color, mvp );
                }

            }

            if ( p.hudActive && p.hud != HudDisplay::kNone ) _drawHUD();
        }


    }


    void GLViewport::_drawAnnotations(const math::Matrix4x4f& mvp)
    {
        TLRENDER_P();
        TLRENDER_GL();
        
        const int64_t frame = p.ui->uiTimeline->value();

        // @todo: extract from attrs in action toolbar.
        int previous = 25;
        int next = 25;
                    
        const std::vector< std::shared_ptr< draw::Annotation > >&
            annotations = _getAnnotationsForFrame( frame, previous, next );
        if ( !annotations.empty() )
        {
            glStencilMask(~0);
            glClear(GL_STENCIL_BUFFER_BIT);
            glEnable( GL_STENCIL_TEST );
            
            for ( const auto& annotation : annotations )
            {
                int64_t annotationFrame = annotation->frame();
                float alphamult = 1.F;
                if ( previous )
                {
                    for ( short i = previous; i > 0; --i )
                    {
                        if ( frame - i == annotationFrame )
                        {
                            alphamult -= (float)i/previous;
                            break;
                        }
                    }
                }
                else if ( next )
                {
                    for ( short i = 1; i <= next; ++i )
                    {
                        if ( frame + i == annotationFrame )
                        {
                            alphamult -= (float)i/next;
                            break;
                        }
                    }
                }
                const auto& shapes = annotation->shapes();
                
                // Shapes are drawn in reverse order, so the erase path works
                ShapeList::const_reverse_iterator i = shapes.rbegin();
                ShapeList::const_reverse_iterator e = shapes.rend();
                
                for ( ; i != e; ++i )
                {
                    const auto& shape = *i;
                    float a = shape->color.a;
                    shape->color.a *= alphamult;
                    shape->matrix = mvp;
                    shape->draw( gl.render );
                    shape->color.a = a;
                }
            }
            glDisable( GL_STENCIL_TEST );
        }
    }
    
    
    inline
    void GLViewport::_drawCropMask( const imaging::Size& renderSize )
    {
        TLRENDER_GL();

        double aspectY = (double) renderSize.w / (double) renderSize.h;
        double aspectX = (double) renderSize.h / (double) renderSize.w;

        double target_aspect = 1.0 / _p->masking;
        double amountY = (0.5 - target_aspect * aspectY / 2);
        double amountX = (0.5 - _p->masking * aspectX / 2);

        bool vertical = true;
        if ( amountY < amountX )
        {
            vertical = false;
        }

        if ( vertical )
        {
            int Y = renderSize.h * amountY;
            imaging::Color4f maskColor( 0, 0, 0, 1 );
            math::BBox2i box( 0, 0, renderSize.w, Y );
            gl.render->drawRect( box, maskColor );
            box.max.y = renderSize.h;
            box.min.y = renderSize.h - Y;
            gl.render->drawRect( box, maskColor );
        }
        else
        {
            int X = renderSize.w * amountX;
            imaging::Color4f maskColor( 0, 0, 0, 1 );
            math::BBox2i box( 0, 0, X, renderSize.h );
            gl.render->drawRect( box, maskColor );
            box.max.x = renderSize.w;
            box.min.x = renderSize.w - X;
            gl.render->drawRect( box, maskColor );
        }

    }

    inline
    void GLViewport::_drawText(
        const std::vector<std::shared_ptr<imaging::Glyph> >& glyphs,
        math::Vector2i& pos, const int16_t lineHeight,
        const imaging::Color4f& labelColor)
    {
        TLRENDER_GL();
        const imaging::Color4f shadowColor(0.F, 0.F, 0.F, 0.7F);
        math::Vector2i shadowPos{ pos.x + 2, pos.y + 2 };
        gl.render->drawText( glyphs, shadowPos, shadowColor );
        gl.render->drawText( glyphs, pos, labelColor );
        pos.y += lineHeight;
    }

    void GLViewport::_getPixelValue(
        imaging::Color4f& rgba,
        const std::shared_ptr<imaging::Image>& image,
        const math::Vector2i& pos ) const
    {
        TLRENDER_P();
        imaging::PixelType type = image->getPixelType();
        uint8_t channels = imaging::getChannelCount(type);
        uint8_t depth    = imaging::getBitDepth(type) / 8;
        const auto& info   = image->getInfo();
        imaging::VideoLevels  videoLevels = info.videoLevels;
        const math::Vector4f& yuvCoefficients =
            getYUVCoefficients( info.yuvCoefficients );
        imaging::Size size = image->getSize();
        const uint8_t*  data = image->getData();
        int X = pos.x;
        int Y = size.h - pos.y - 1;
        if ( p.displayOptions[0].mirror.x ) X = size.w - X - 1;
        if ( p.displayOptions[0].mirror.y ) Y = size.h - Y - 1;

        // Do some sanity check just in case
        if ( X < 0 || Y < 0 || X >= size.w || Y >= size.h )
            return;

        size_t offset = ( Y * size.w + X ) * depth;

        switch( type )
        {
        case imaging::PixelType::YUV_420P_U8:
        case imaging::PixelType::YUV_422P_U8:
        case imaging::PixelType::YUV_444P_U8:
            break;
        case imaging::PixelType::YUV_420P_U16:
        case imaging::PixelType::YUV_422P_U16:
        case imaging::PixelType::YUV_444P_U16:
            break;
        default:
            offset *= channels;
            break;
        }

        rgba.a = 1.0f;
        switch ( type )
        {
        case imaging::PixelType::L_U8:
            rgba.r = data[offset] / 255.0f;
            rgba.g = data[offset] / 255.0f;
            rgba.b = data[offset] / 255.0f;
            break;
        case imaging::PixelType::LA_U8:
            rgba.r = data[offset]   / 255.0f;
            rgba.g = data[offset]   / 255.0f;
            rgba.b = data[offset]   / 255.0f;
            rgba.a = data[offset+1] / 255.0f;
            break;
        case imaging::PixelType::L_U16:
        {
            uint16_t* f = (uint16_t*) (&data[offset]);
            rgba.r = f[0] / 65535.0f;
            rgba.g = f[0] / 65535.0f;
            rgba.b = f[0] / 65535.0f;
            break;
        }
        case imaging::PixelType::LA_U16:
        {
            uint16_t* f = (uint16_t*) (&data[offset]);
            rgba.r = f[0] / 65535.0f;
            rgba.g = f[0] / 65535.0f;
            rgba.b = f[0] / 65535.0f;
            rgba.a = f[1] / 65535.0f;
            break;
        }
        case imaging::PixelType::L_U32:
        {
            uint32_t* f = (uint32_t*) (&data[offset]);
            constexpr float max = static_cast<float>(
                std::numeric_limits<uint32_t>::max() );
            rgba.r = f[0] / max;
            rgba.g = f[0] / max;
            rgba.b = f[0] / max;
            break;
        }
        case imaging::PixelType::LA_U32:
        {
            uint32_t* f = (uint32_t*) (&data[offset]);
            constexpr float max = static_cast<float>(
                std::numeric_limits<uint32_t>::max() );
            rgba.r = f[0] / max;
            rgba.g = f[0] / max;
            rgba.b = f[0] / max;
            rgba.a = f[1] / max;
            break;
        }
        case imaging::PixelType::L_F16:
        {
            half* f = (half*) (&data[offset]);
            rgba.r = f[0];
            rgba.g = f[0];
            rgba.b = f[0];
            break;
        }
        case imaging::PixelType::LA_F16:
        {
            half* f = (half*) (&data[offset]);
            rgba.r = f[0];
            rgba.g = f[0];
            rgba.b = f[0];
            rgba.a = f[1];
            break;
        }
        case imaging::PixelType::RGB_U8:
            rgba.r = data[offset] / 255.0f;
            rgba.g = data[offset+1] / 255.0f;
            rgba.b = data[offset+2] / 255.0f;
            break;
        case imaging::PixelType::RGB_U10:
        {
            imaging::U10* f = (imaging::U10*) (&data[offset]);
            constexpr float max = static_cast<float>(
                std::numeric_limits<uint32_t>::max() );
            rgba.r = f->r / max;
            rgba.g = f->g / max;
            rgba.b = f->b / max;
            break;
        }
        case imaging::PixelType::RGBA_U8:
            rgba.r = data[offset] / 255.0f;
            rgba.g = data[offset+1] / 255.0f;
            rgba.b = data[offset+2] / 255.0f;
            rgba.a = data[offset+3] / 255.0f;
            break;
        case imaging::PixelType::RGB_U16:
        {
            uint16_t* f = (uint16_t*) (&data[offset]);
            rgba.r = f[0] / 65535.0f;
            rgba.g = f[1] / 65535.0f;
            rgba.b = f[2] / 65535.0f;
            break;
        }
        case imaging::PixelType::RGBA_U16:
        {
            uint16_t* f = (uint16_t*) (&data[offset]);
            rgba.r = f[0] / 65535.0f;
            rgba.g = f[1] / 65535.0f;
            rgba.b = f[2] / 65535.0f;
            rgba.a = f[3] / 65535.0f;
            break;
        }
        case imaging::PixelType::RGB_U32:
        {
            uint32_t* f = (uint32_t*) (&data[offset]);
            constexpr float max = static_cast<float>(
                std::numeric_limits<uint32_t>::max() );
            rgba.r = f[0] / max;
            rgba.g = f[1] / max;
            rgba.b = f[2] / max;
            break;
        }
        case imaging::PixelType::RGBA_U32:
        {
            uint32_t* f = (uint32_t*) (&data[offset]);
            constexpr float max = static_cast<float>(
                std::numeric_limits<uint32_t>::max() );
            rgba.r = f[0] / max;
            rgba.g = f[1] / max;
            rgba.b = f[2] / max;
            rgba.a = f[3] / max;
            break;
        }
        case imaging::PixelType::RGB_F16:
        {
            half* f = (half*) (&data[offset]);
            rgba.r = f[0];
            rgba.g = f[1];
            rgba.b = f[2];
            break;
        }
        case imaging::PixelType::RGBA_F16:
        {
            half* f = (half*) (&data[offset]);
            rgba.r = f[0];
            rgba.g = f[1];
            rgba.b = f[2];
            rgba.a = f[3];
            break;
        }
        case imaging::PixelType::RGB_F32:
        {
            float* f = (float*) (&data[offset]);
            rgba.r = f[0];
            rgba.g = f[1];
            rgba.b = f[2];
            break;
        }
        case imaging::PixelType::RGBA_F32:
        {
            float* f = (float*) (&data[offset]);
            rgba.r = f[0];
            rgba.g = f[1];
            rgba.b = f[2];
            rgba.a = f[3];
            break;
        }
        case imaging::PixelType::YUV_420P_U8:
        {
            size_t Ysize = size.w * size.h;
            size_t w2      = (size.w + 1) / 2;
            size_t h2      = (size.h + 1) / 2;
            size_t Usize   = w2 * h2;
            size_t offset2 = (Y/2) * w2 + X / 2;
            rgba.r = data[ offset ]                  / 255.0f;
            rgba.g = data[ Ysize + offset2 ]         / 255.0f;
            rgba.b = data[ Ysize + Usize + offset2 ] / 255.0f;
            color::checkLevels( rgba, videoLevels );
            rgba = color::YPbPr::to_rgb( rgba, yuvCoefficients );
            break;
        }
        case imaging::PixelType::YUV_422P_U8:
        {
            size_t Ysize = size.w * size.h;
            size_t w2      = (size.w + 1) / 2;
            size_t Usize   = w2 * size.h;
            size_t offset2 = Y * w2 + X / 2;
            rgba.r = data[ offset ]              / 255.0f;
            rgba.g = data[ Ysize + offset2 ]         / 255.0f;
            rgba.b = data[ Ysize + Usize + offset2 ] / 255.0f;
            color::checkLevels( rgba, videoLevels );
            rgba = color::YPbPr::to_rgb( rgba, yuvCoefficients );
            break;
        }
        case imaging::PixelType::YUV_444P_U8:
        {
            size_t Ysize = size.w * size.h;
            rgba.r = data[ offset ]             / 255.0f;
            rgba.g = data[ Ysize + offset ]     / 255.0f;
            rgba.b = data[ Ysize * 2 + offset ] / 255.0f;
            color::checkLevels( rgba, videoLevels );
            rgba = color::YPbPr::to_rgb( rgba, yuvCoefficients );
            break;
        }
        case imaging::PixelType::YUV_420P_U16:
        {
            size_t pos = Y * size.w / 4 + X / 2;
            size_t Ysize = size.w * size.h;
            size_t Usize = Ysize / 4;
            rgba.r = data[ offset ]              / 65535.0f;
            rgba.g = data[ Ysize + pos ]         / 65535.0f;
            rgba.b = data[ Ysize + Usize + pos ] / 65535.0f;
            color::checkLevels( rgba, videoLevels );
            rgba = color::YPbPr::to_rgb( rgba, yuvCoefficients );
            break;
        }
        case imaging::PixelType::YUV_422P_U16:
        {
            size_t Ysize = size.w * size.h * depth;
            size_t pos = Y * size.w + X;
            size_t Usize = size.w / 2 * size.h * depth;
            rgba.r = data[ offset ]              / 65535.0f;
            rgba.g = data[ Ysize + pos ]         / 65535.0f;
            rgba.b = data[ Ysize + Usize + pos ] / 65535.0f;
            color::checkLevels( rgba, videoLevels );
            rgba = color::YPbPr::to_rgb( rgba, yuvCoefficients );
            break;
        }
        case imaging::PixelType::YUV_444P_U16:
        {
            size_t Ysize = size.w * size.h * depth;
            rgba.r = data[ offset ]             / 65535.0f;
            rgba.g = data[ Ysize + offset ]     / 65535.0f;
            rgba.b = data[ Ysize * 2 + offset ] / 65535.0f;
            color::checkLevels( rgba, videoLevels );
            rgba = color::YPbPr::to_rgb( rgba, yuvCoefficients );
            break;
        }
        default:
            break;
        }

    }

    void GLViewport::calculateColorAreaInfo( const math::BBox2i& box, mrv::area::Info& info )
    {
        TLRENDER_P();
        TLRENDER_GL();

        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_PACK_SWAP_BYTES, GL_FALSE );

        const GLenum format = GL_BGRA;  // for faster access, we muse use BGRA.
        const GLenum type = GL_FLOAT;
        const imaging::Size& renderSize = gl.buffer->getSize();

        // set the target framebuffer to read
        gl::OffscreenBufferBinding binding(gl.buffer);
        // "index" is used to read pixels from framebuffer to a PBO
        // "nextIndex" is used to update pixels in the other PBO
        gl.index = (gl.index + 1) % 2;
        gl.nextIndex = (gl.index + 1) % 2;

        // read pixels from framebuffer to PBO
        // glReadPixels() should return immediately.
        glBindBuffer(GL_PIXEL_PACK_BUFFER, gl.pboIds[gl.index]);
        glReadPixels(0, 0, renderSize.w, renderSize.h, format, type, 0);

        // map the PBO to process its data by CPU
        glBindBuffer(GL_PIXEL_PACK_BUFFER, gl.pboIds[gl.nextIndex]);
        gl.image = (GLfloat*)glMapBuffer(GL_PIXEL_PACK_BUFFER,
                                             GL_READ_ONLY);
        if(gl.image)
        {
            BrightnessType brightness_type =
                (BrightnessType) p.ui->uiLType->value();
            info.rgba.max.r = std::numeric_limits<float>::min();
            info.rgba.max.g = std::numeric_limits<float>::min();
            info.rgba.max.b = std::numeric_limits<float>::min();
            info.rgba.max.a = std::numeric_limits<float>::min();

            info.rgba.min.r = std::numeric_limits<float>::max();
            info.rgba.min.g = std::numeric_limits<float>::max();
            info.rgba.min.b = std::numeric_limits<float>::max();
            info.rgba.min.a = std::numeric_limits<float>::max();

            info.rgba.mean.r = info.rgba.mean.g = info.rgba.mean.b =
            info.rgba.mean.a = 0.F;


            info.hsv.max.r = std::numeric_limits<float>::min();
            info.hsv.max.g = std::numeric_limits<float>::min();
            info.hsv.max.b = std::numeric_limits<float>::min();
            info.hsv.max.a = std::numeric_limits<float>::min();

            info.hsv.min.r = std::numeric_limits<float>::max();
            info.hsv.min.g = std::numeric_limits<float>::max();
            info.hsv.min.b = std::numeric_limits<float>::max();
            info.hsv.min.a = std::numeric_limits<float>::max();

            info.hsv.mean.r = info.hsv.mean.g = info.hsv.mean.b =
            info.hsv.mean.a = 0.F;

            int hsv_colorspace = p.ui->uiBColorType->value() + 1;

            int maxX = box.max.x;
            int maxY = box.max.y;
            for ( int Y = box.y(); Y < maxY; ++Y )
            {
                for ( int X = box.x(); X < maxX; ++X )
                {
                    imaging::Color4f rgba, hsv;
                    rgba.b = gl.image[ ( X + Y * renderSize.w ) * 4 ];
                    rgba.g = gl.image[ ( X + Y * renderSize.w ) * 4 + 1 ];
                    rgba.r = gl.image[ ( X + Y * renderSize.w ) * 4 + 2 ];
                    rgba.a = gl.image[ ( X + Y * renderSize.w ) * 4 + 3 ];


                    info.rgba.mean.r += rgba.r;
                    info.rgba.mean.g += rgba.g;
                    info.rgba.mean.b += rgba.b;
                    info.rgba.mean.a += rgba.a;

                    if ( rgba.r < info.rgba.min.r ) info.rgba.min.r = rgba.r;
                    if ( rgba.g < info.rgba.min.g ) info.rgba.min.g = rgba.g;
                    if ( rgba.b < info.rgba.min.b ) info.rgba.min.b = rgba.b;
                    if ( rgba.a < info.rgba.min.a ) info.rgba.min.a = rgba.a;

                    if ( rgba.r > info.rgba.max.r ) info.rgba.max.r = rgba.r;
                    if ( rgba.g > info.rgba.max.g ) info.rgba.max.g = rgba.g;
                    if ( rgba.b > info.rgba.max.b ) info.rgba.max.b = rgba.b;
                    if ( rgba.a > info.rgba.max.a ) info.rgba.max.a = rgba.a;

                    if ( rgba.r < 0 ) rgba.r = 0;
                    if ( rgba.g < 0 ) rgba.g = 0;
                    if ( rgba.b < 0 ) rgba.b = 0;
                    if ( rgba.r > 1 ) rgba.r = 1.F;
                    if ( rgba.g > 1 ) rgba.g = 1.F;
                    if ( rgba.b > 1 ) rgba.b = 1.F;

                    switch( hsv_colorspace )
                    {
                    case color::kHSV:
                        hsv = color::rgb::to_hsv( rgba );
                        break;
                    case color::kHSL:
                        hsv = color::rgb::to_hsl( rgba );
                        break;
                    case color::kCIE_XYZ:
                        hsv = color::rgb::to_xyz( rgba );
                        break;
                    case color::kCIE_xyY:
                        hsv = color::rgb::to_xyY( rgba );
                        break;
                    case color::kCIE_Lab:
                        hsv = color::rgb::to_lab( rgba );
                        break;
                    case color::kCIE_Luv:
                        hsv = color::rgb::to_luv( rgba );
                        break;
                    case color::kYUV:
                        hsv = color::rgb::to_yuv( rgba );
                        break;
                    case color::kYDbDr:
                        hsv = color::rgb::to_YDbDr( rgba );
                        break;
                    case color::kYIQ:
                        hsv = color::rgb::to_yiq( rgba );
                        break;
                    case color::kITU_601:
                        hsv = color::rgb::to_ITU601( rgba );
                        break;
                    case color::kITU_709:
                        hsv = color::rgb::to_ITU709( rgba );
                        break;
                    case color::kRGB:
                    default:
                        hsv = rgba;
                        break;
                    }
                    hsv.a = calculate_brightness( rgba, brightness_type );

                    info.hsv.mean.r += hsv.r;
                    info.hsv.mean.g += hsv.g;
                    info.hsv.mean.b += hsv.b;
                    info.hsv.mean.a += hsv.a;

                    if ( hsv.r < info.hsv.min.r ) info.hsv.min.r = hsv.r;
                    if ( hsv.g < info.hsv.min.g ) info.hsv.min.g = hsv.g;
                    if ( hsv.b < info.hsv.min.b ) info.hsv.min.b = hsv.b;
                    if ( hsv.a < info.hsv.min.a ) info.hsv.min.a = hsv.a;

                    if ( hsv.r > info.hsv.max.r ) info.hsv.max.r = hsv.r;
                    if ( hsv.g > info.hsv.max.g ) info.hsv.max.g = hsv.g;
                    if ( hsv.b > info.hsv.max.b ) info.hsv.max.b = hsv.b;
                    if ( hsv.a > info.hsv.max.a ) info.hsv.max.a = hsv.a;
                }
            }

            int num = box.w() * box.h();
            assert( num > 0 );
            info.rgba.mean.r /= num;
            info.rgba.mean.g /= num;
            info.rgba.mean.b /= num;
            info.rgba.mean.a /= num;

            info.rgba.diff.r = info.rgba.max.r - info.rgba.min.r;
            info.rgba.diff.g = info.rgba.max.g - info.rgba.min.g;
            info.rgba.diff.b = info.rgba.max.b - info.rgba.min.b;
            info.rgba.diff.a = info.rgba.max.a - info.rgba.min.a;

            info.hsv.mean.r /= num;
            info.hsv.mean.g /= num;
            info.hsv.mean.b /= num;
            info.hsv.mean.a /= num;

            info.hsv.diff.r = info.hsv.max.r - info.hsv.min.r;
            info.hsv.diff.g = info.hsv.max.g - info.hsv.min.g;
            info.hsv.diff.b = info.hsv.max.b - info.hsv.min.b;
            info.hsv.diff.a = info.hsv.max.a - info.hsv.min.a;
        }
    }


    void GLViewport::_readPixel( imaging::Color4f& rgba ) const noexcept
    {
        if ( !valid() ) return;

        TLRENDER_P();
        TLRENDER_GL();

        math::Vector2i pos;
        pos.x = ( p.mousePos.x - p.viewPos.x ) / p.viewZoom;
        pos.y = ( p.mousePos.y - p.viewPos.y ) / p.viewZoom;

        if ( p.ui->uiPixelValue->value() != PixelValue::kFull )
        {

            rgba.r = rgba.g = rgba.b = rgba.a = 0.f;

            for ( const auto& video : p.videoData )
            {
                for ( const auto& layer : video.layers )
                {
                    const auto& image = layer.image;
                    if ( ! image->isValid() ) continue;

                    imaging::Color4f pixel, pixelB;

                    _getPixelValue( pixel, image, pos );


                    const auto& imageB = layer.image;
                    if ( imageB->isValid() )
                    {
                        _getPixelValue( pixelB, imageB, pos );

                        if ( layer.transition ==
                             timeline::Transition::Dissolve )
                        {
                            float f2 = layer.transitionValue;
                            float  f = 1.0 - f2;
                            pixel.r = pixel.r * f + pixelB.r * f2;
                            pixel.g = pixel.g * f + pixelB.g * f2;
                            pixel.b = pixel.b * f + pixelB.b * f2;
                            pixel.a = pixel.a * f + pixelB.a * f2;
                        }
                    }
                    rgba.r += pixel.r;
                    rgba.g += pixel.g;
                    rgba.b += pixel.b;
                    rgba.a += pixel.a;
                }
            }
        }
        else
        {

            glPixelStorei(GL_PACK_ALIGNMENT, 1);
            glPixelStorei(GL_PACK_SWAP_BYTES, GL_FALSE );

            gl::OffscreenBufferBinding binding(gl.buffer);


            const GLenum format = GL_BGRA;  // for faster access, we muse use BGRA.
            const GLenum type = GL_FLOAT;

            if ( !p.timelinePlayers.empty() &&
                 p.timelinePlayers[0]->playback() == timeline::Playback::Stop )
            {
                glReadPixels( pos.x, pos.y, 1, 1, GL_RGBA, type, &rgba);
                return;
            }



            const imaging::Size& renderSize = gl.buffer->getSize();

            if ( ! gl.image )
            {
                // set the target framebuffer to read
                // "index" is used to read pixels from framebuffer to a PBO
                // "nextIndex" is used to update pixels in the other PBO
                gl.index = (gl.index + 1) % 2;
                gl.nextIndex = (gl.index + 1) % 2;

                // read pixels from framebuffer to PBO
                // glReadPixels() should return immediately.
                glBindBuffer(GL_PIXEL_PACK_BUFFER, gl.pboIds[gl.index]);
                glReadPixels(0, 0, renderSize.w, renderSize.h, format, type, 0);

                // map the PBO to process its data by CPU
                glBindBuffer(GL_PIXEL_PACK_BUFFER, gl.pboIds[gl.nextIndex]);

                gl.image = (GLfloat*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
            }

            if( gl.image )
            {
                rgba.b = gl.image[ ( pos.x + pos.y * renderSize.w ) * 4 ];
                rgba.g = gl.image[ ( pos.x + pos.y * renderSize.w ) * 4 + 1 ];
                rgba.r = gl.image[ ( pos.x + pos.y * renderSize.w ) * 4 + 2 ];
                rgba.a = gl.image[ ( pos.x + pos.y * renderSize.w ) * 4 + 3 ];
            }
        }

        if ( gl.image )
        {
            glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
            gl.image = nullptr;
        }

        // back to conventional pixel operations
        glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    }

    bool GLViewport::getHudActive() const
    {
        return _p->hudActive;
    }

    void GLViewport::setHudActive( const bool active )
    {
        _p->hudActive = active;
        redraw();
    }

    void GLViewport::setHudDisplay( const HudDisplay hud )
    {
        _p->hud = hud;
        redraw();
    }

    HudDisplay GLViewport::getHudDisplay() const noexcept
    {
        return _p->hud;
    }


    void GLViewport::_drawHUD()
    {
        TLRENDER_P();
        TLRENDER_GL();


        const auto& viewportSize = _getViewportSize();

        timeline::RenderOptions renderOptions;
        renderOptions.clear = false;

        gl.render->begin( viewportSize, renderOptions );

        std::string fontFamily = "NotoSans-Regular";
        uint16_t fontSize = 12;

        Fl_Color c = p.ui->uiPrefs->uiPrefsViewHud->color();
        uint8_t r, g, b;
        Fl::get_color( c, r, g, b );

        const imaging::Color4f labelColor(r / 255.F, g / 255.F, b / 255.F);

        char buf[128];
        const imaging::FontInfo fontInfo(fontFamily, fontSize);
        const imaging::FontMetrics fontMetrics = p.fontSystem->getMetrics(fontInfo);
        auto lineHeight = fontMetrics.lineHeight;
        math::Vector2i pos( 20, lineHeight*2 );

        const auto& player = p.timelinePlayers[0];
        const auto& path   = player->path();
        const otime::RationalTime& time = player->currentTime();
        int64_t frame = time.to_frames();

        const auto& directory = path.getDirectory();

        std::string fullname = createStringFromPathAndTime( path, time );

        if ( p.hud & HudDisplay::kDirectory )
            _drawText( p.fontSystem->getGlyphs(directory, fontInfo),
                       pos, lineHeight, labelColor );

        if ( p.hud & HudDisplay::kFilename )
            _drawText( p.fontSystem->getGlyphs(fullname, fontInfo), pos,
                       lineHeight, labelColor );

        if ( p.hud & HudDisplay::kResolution )
        {
            const auto& info   = player->timelinePlayer()->getIOInfo();
            const auto& video = info.video[0];
            if ( video.size.pixelAspectRatio != 1.0 )
            {
                int width = video.size.w * video.size.pixelAspectRatio;
                sprintf( buf, "%d x %d  ( %.3g )  %d x %d",
                         video.size.w, video.size.h,
                         video.size.pixelAspectRatio, width, video.size.h);
            }
            else
            {
                sprintf( buf, "%d x %d", video.size.w, video.size.h );
            }
            _drawText( p.fontSystem->getGlyphs(buf, fontInfo), pos,
                       lineHeight, labelColor );
        }

        const otime::TimeRange&    range = player->timeRange();
        const otime::RationalTime& duration = range.end_time_inclusive() -
                                              range.start_time();

        std::string tmp;
        if ( p.hud & HudDisplay::kFrame )
        {
            sprintf( buf, "F: %" PRId64 " ", frame );
            tmp += buf;
        }

        if ( p.hud & HudDisplay::kFrameRange )
        {
            const auto& range = player->timeRange();
            frame = range.start_time().to_frames();
            const int64_t last_frame = range.end_time_inclusive().to_frames();
            sprintf( buf, "Range: %" PRId64 " -  %" PRId64,
                     frame, last_frame );
            tmp += buf;
        }

        if ( p.hud & HudDisplay::kTimecode )
        {
          sprintf( buf, "TC: %s ", time.to_timecode(nullptr).c_str() );
          tmp += buf;
        }

        if ( p.hud & HudDisplay::kFPS )
        {
            sprintf( buf, "FPS: %.3f", p.ui->uiFPS->value() );
            tmp += buf;
        }

        if ( !tmp.empty() )
            _drawText( p.fontSystem->getGlyphs(tmp, fontInfo), pos,
                       lineHeight, labelColor );

        tmp.clear();
        if ( p.hud & HudDisplay::kFrameCount )
        {
            sprintf( buf, "FC: %" PRId64, (int64_t)duration.value() );
            tmp += buf;
        }


        if ( !tmp.empty() )
            _drawText( p.fontSystem->getGlyphs(tmp, fontInfo), pos,
                       lineHeight, labelColor );

        if ( p.hud & HudDisplay::kAttributes )
        {
            const auto& info   = player->timelinePlayer()->getIOInfo();
            for ( auto& tag : info.tags )
            {
                sprintf( buf, "%s = %s",
                         tag.first.c_str(), tag.second.c_str() );
                _drawText( p.fontSystem->getGlyphs(buf, fontInfo), pos,
                           lineHeight, labelColor );
            }
        }

    }
}
