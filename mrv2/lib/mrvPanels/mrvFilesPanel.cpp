// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <string>
#include <vector>
#include <map>

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvPack.h"
#include "mrvWidgets/mrvFileButton.h"
#include "mrvWidgets/mrvButton.h"

#include "mrvPanels/mrvPanelsAux.h"
#include "mrvPanels/mrvPanelsCallbacks.h"
#include "mrvPanels/mrvFilesPanel.h"

#include "mrvGL/mrvThumbnailCreator.h"

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/App.h"

#include "mrViewer.h"

#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "files";
}

namespace mrv
{
    typedef std::map< FileButton*, int64_t > WidgetIds;
    typedef std::map< FileButton*, size_t > WidgetIndices;

    struct FilesPanel::Private
    {
        std::weak_ptr<system::Context> context;
        mrv::ThumbnailCreator* thumbnailCreator;
        App* app;
        std::map< size_t, FileButton* > map;
        WidgetIds ids;
        WidgetIndices indices;
        std::vector< Fl_Button* > buttons;

        std::shared_ptr<
            observer::ListObserver<std::shared_ptr<FilesModelItem> > >
            filesObserver;

        std::shared_ptr<observer::ValueObserver<int> > aIndexObserver;
        std::shared_ptr<observer::ListObserver<int> > layerObserver;
    };

    struct ThumbnailData
    {
        FileButton* widget;
    };

    void filesThumbnail_cb(
        const int64_t id,
        const std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> >&
            thumbnails,
        void* opaque)
    {
        ThumbnailData* data = static_cast< ThumbnailData* >(opaque);
        FileButton* w = data->widget;
        if (filesPanel)
            filesPanel->filesThumbnail(id, thumbnails, w);
        delete data;
    }

    void FilesPanel::filesThumbnail(
        const int64_t id,
        const std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> >&
            thumbnails,
        FileButton* w)
    {

        WidgetIds::const_iterator it = _r->ids.find(w);
        if (it == _r->ids.end())
            return;

        if (it->second == id)
        {

            for (const auto& i : thumbnails)
            {
                Fl_Image* img = w->image();
                w->image(i.second);
                delete img;
                w->redraw();
            }
        }
        else
        {

            for (const auto& i : thumbnails)
            {

                delete i.second;
            }
        }
    }

    FilesPanel::FilesPanel(ViewerUI* ui) :
        _r(new Private),
        PanelWidget(ui)
    {
        _r->context = ui->app->getContext();

        add_group("Files");

        Fl_SVG_Image* svg = load_svg("Files.svg");
        g->image(svg);

        g->callback(
            [](Fl_Widget* w, void* d)
            {
                ViewerUI* ui = static_cast< ViewerUI* >(d);
                delete filesPanel;
                filesPanel = nullptr;
                ui->uiMain->fill_menu(ui->uiMenuBar);
            },
            ui);

        _r->filesObserver =
            observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                ui->app->filesModel()->observeFiles(),
                [this](
                    const std::vector< std::shared_ptr<FilesModelItem> >& value)
                { refresh(); });

        _r->aIndexObserver = observer::ValueObserver<int>::create(
            ui->app->filesModel()->observeAIndex(),
            [this](int value) { redraw(); });

        _r->layerObserver = observer::ListObserver<int>::create(
            ui->app->filesModel()->observeLayers(),
            [this](const std::vector<int>& value) { redraw(); });
    }

    FilesPanel::~FilesPanel()
    {
        cancel_thumbnails();
        clear_controls();
    }

    void FilesPanel::cancel_thumbnails()
    {
        for (const auto& it : _r->ids)
        {
            _r->thumbnailCreator->cancelRequests(it.second);
        }

        _r->ids.clear();
    }

    void FilesPanel::clear_controls()
    {
        for (const auto& i : _r->map)
        {
            Fl_Button* b = i.second;

            delete b->image();
            b->image(nullptr);
            g->remove(b);
            delete b;
        }

        // Clear buttons' SVG images
        for (const auto& b : _r->buttons)
        {
            delete b->image();
            b->image(nullptr);
        }

        _r->buttons.clear();
        _r->map.clear();
        _r->indices.clear();
    }

    void FilesPanel::add_controls()
    {
        TLRENDER_P();

        Fl_SVG_Image* svg;
        _r->thumbnailCreator = p.ui->uiTimeline->thumbnailCreator();
        if (!_r->thumbnailCreator)
            return;

        g->clear();

        g->begin();

        const auto model = p.ui->app->filesModel();

        const auto files = model->observeFiles();

        size_t numFiles = files->getSize();

        auto Aindex = model->observeAIndex()->get();

        const auto player = p.ui->uiView->getTimelinePlayer();

        otio::RationalTime time = otio::RationalTime(0.0, 1.0);
        if (player)
            time = player->currentTime();

        image::Size size(128, 64);

        for (size_t i = 0; i < numFiles; ++i)
        {
            const auto& media = files->getItem(i);
            const auto& path = media->path;

            const std::string& dir = path.getDirectory();
            const std::string file =
                path.getBaseName() + path.getNumber() + path.getExtension();
            const std::string fullfile = dir + file;

            auto bW = new Widget<FileButton>(
                g->x(), g->y() + 22 + i * 68, g->w(), 68);
            FileButton* b = bW;
            _r->indices[b] = i;
            b->tooltip(_("Select main A image."));
            bW->callback(
                [=](auto b)
                {
                    WidgetIndices::const_iterator it = _r->indices.find(b);
                    if (it == _r->indices.end())
                        return;
                    int index = (*it).second;
                    auto model = _p->ui->app->filesModel();
                    model->setA(index);
                });

            _r->map[i] = b;

            time = media->currentTime;
            uint16_t layerId = media->videoLayer;
            if (Aindex == i)
            {
                b->value(1);
                if (player)
                {
                    time = player->currentTime();
                    layerId = p.ui->uiColorChannel->value();
                }
            }
            else
            {
                b->value(0);
            }

            const std::string& layer = getLayerName(layerId, p.ui);
            std::string text = dir + "\n" + file + layer;
            b->copy_label(text.c_str());

            if (auto context = _r->context.lock())
            {

                ThumbnailData* data = new ThumbnailData;
                data->widget = b;

                WidgetIds::const_iterator it = _r->ids.find(b);
                if (it != _r->ids.end())
                {
                    _r->thumbnailCreator->cancelRequests(it->second);
                    _r->ids.erase(it);
                }

                try
                {
                    auto timeline =
                        timeline::Timeline::create(path.get(), context);
                    auto timeRange = timeline->getTimeRange();

                    if (time::isValid(timeRange))
                    {
                        auto startTime = timeRange.start_time();
                        auto endTime = timeRange.end_time_inclusive();

                        if (time < startTime)
                            time = startTime;
                        else if (time > endTime)
                            time = endTime;
                    }

                    _r->thumbnailCreator->initThread();

                    int64_t id = _r->thumbnailCreator->request(
                        fullfile, time, size, filesThumbnail_cb, (void*)data,
                        layerId);
                    _r->ids[b] = id;
                }
                catch (const std::exception& e)
                {
                }
            }
        }

        int Y = g->y() + 20 + numFiles * 64;

        Pack* bg = new Pack(g->x(), Y, g->w(), 30);
        bg->type(Pack::HORIZONTAL);
        bg->begin();

        Fl_Button* b;
        auto bW = new Widget< Button >(g->x(), Y, 30, 30);
        b = bW;

        svg = load_svg("FileOpen.svg");
        b->image(svg);

        _r->buttons.push_back(b);

        b->tooltip(_("Open a filename"));
        bW->callback([=](auto w) { open_cb(w, p.ui); });

        bW = new Widget< Button >(g->x() + 30, Y, 30, 30);
        b = bW;
        svg = load_svg("FileOpenSeparateAudio.svg");
        b->image(svg);
        _r->buttons.push_back(b);
        b->tooltip(_("Open a filename with audio"));
        bW->callback([=](auto w) { open_separate_audio_cb(w, p.ui); });

        bW = new Widget< Button >(g->x() + 60, Y, 30, 30);
        b = bW;
        svg = load_svg("FileClose.svg");
        b->image(svg);
        _r->buttons.push_back(b);
        b->tooltip(_("Close current filename"));
        bW->callback([=](auto w) { close_current_cb(w, p.ui); });

        bW = new Widget< Button >(g->x() + 90, Y, 30, 30);
        b = bW;
        svg = load_svg("FileCloseAll.svg");
        b->image(svg);
        _r->buttons.push_back(b);
        b->tooltip(_("Close all filenames"));
        bW->callback([=](auto w) { close_all_cb(w, p.ui); });

        bW = new Widget< Button >(g->x() + 120, Y, 30, 30);
        b = bW;
        svg = load_svg("Prev.svg");
        b->image(svg);
        _r->buttons.push_back(b);
        b->tooltip(_("Previous filename"));
        bW->callback(
            [=](auto w)
            {
                p.ui->app->filesModel()->prev();
                redraw();
            });

        bW = new Widget< Button >(g->x() + 150, Y, 30, 30);
        b = bW;
        svg = load_svg("Next.svg");
        b->image(svg);
        _r->buttons.push_back(b);
        b->tooltip(_("Next filename"));
        bW->callback(
            [=](auto w)
            {
                p.ui->app->filesModel()->next();
                redraw();
            });

        bg->end();
        g->layout();

        g->end();
    }

    void FilesPanel::redraw()
    {

        TLRENDER_P();

        otio::RationalTime time = otio::RationalTime(0.0, 1.0);

        const auto player = p.ui->uiView->getTimelinePlayer();

        image::Size size(128, 64);

        const auto& model = p.ui->app->filesModel();
        auto Aindex = model->observeAIndex()->get();
        const auto files = model->observeFiles();

        for (auto& m : _r->map)
        {
            size_t i = m.first;
            const auto& media = files->getItem(i);
            const auto& path = media->path;

            const std::string& dir = path.getDirectory();
            const std::string file =
                path.getBaseName() + path.getNumber() + path.getExtension();
            const std::string fullfile = dir + file;
            FileButton* b = m.second;

            const std::string& layer = getLayerName(media->videoLayer, p.ui);
            std::string text = dir + "\n" + file + layer;
            b->copy_label(text.c_str());

            b->labelcolor(FL_WHITE);
            WidgetIndices::iterator it = _r->indices.find(b);
            time = media->currentTime;
            uint16_t layerId = media->videoLayer;
            if (Aindex != i)
            {
                b->value(0);
                if (b->image())
                    continue;
            }
            else
            {
                b->value(1);
                if (player)
                {
                    time = player->currentTime();
                    layerId = p.ui->uiColorChannel->value();
                }
            }

            if (auto context = _r->context.lock())
            {
                ThumbnailData* data = new ThumbnailData;
                data->widget = b;

                WidgetIds::const_iterator it = _r->ids.find(b);
                if (it != _r->ids.end())
                {
                    _r->thumbnailCreator->cancelRequests(it->second);
                    _r->ids.erase(it);
                }

                try
                {
                    auto timeline =
                        timeline::Timeline::create(fullfile, context);
                    auto timeRange = timeline->getTimeRange();

                    if (time::isValid(timeRange))
                    {
                        auto startTime = timeRange.start_time();
                        auto endTime = timeRange.end_time_inclusive();

                        if (time < startTime)
                            time = startTime;
                        else if (time > endTime)
                            time = endTime;
                    }

                    _r->thumbnailCreator->initThread();

                    int64_t id = _r->thumbnailCreator->request(
                        fullfile, time, size, filesThumbnail_cb, (void*)data,
                        layerId);
                    _r->ids[b] = id;
                }
                catch (const std::exception& e)
                {
                }
            }
        }
    }

    void FilesPanel::refresh()
    {
        cancel_thumbnails();
        clear_controls();
        add_controls();
        end_group();
    }

} // namespace mrv
