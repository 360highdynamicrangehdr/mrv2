// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <limits>
#include <cmath>
#include <vector>
#include <iostream>

#include <tlCore/Vector.h>
#include <tlCore/Matrix.h>

#include <tlTimeline/IRender.h>

#include "mrvDraw/Point.h"
#include "mrvGL/mrvGLLines.h"

namespace tl
{

    namespace draw
    {

        class Shape
        {
        public:
            Shape() :
                color(0.F, 1.F, 0.F, 1.F),
                soft(false),
                laser(false),
                fade(1.0F),
                pen_size(5){};

            virtual ~Shape(){};

            virtual void draw(
                const std::shared_ptr<timeline::IRender>&,
                const std::shared_ptr<gl::Lines>&) = 0;

        public:
            math::Matrix4x4f matrix;
            image::Color4f color;
            bool soft;
            bool laser;
            float fade;
            float pen_size;
        };

        class PathShape : public Shape
        {
        public:
            PathShape() :
                Shape(){};
            virtual ~PathShape(){};

            virtual void draw(
                const std::shared_ptr<timeline::IRender>&,
                const std::shared_ptr<gl::Lines>&) = 0;

            PointList pts;
        };

        class NoteShape : public Shape
        {
        public:
            NoteShape() :
                Shape(){};
            virtual ~NoteShape(){};

            void draw(
                const std::shared_ptr<timeline::IRender>&,
                const std::shared_ptr<gl::Lines>&) override{};

        public:
            std::string text;
        };

        void to_json(nlohmann::json&, const Shape&);

        void from_json(const nlohmann::json&, Shape&);

        void to_json(nlohmann::json&, const PathShape&);

        void from_json(const nlohmann::json&, PathShape&);

        void to_json(nlohmann::json&, const NoteShape&);

        void from_json(const nlohmann::json&, NoteShape&);

    } // namespace draw
} // namespace tl
