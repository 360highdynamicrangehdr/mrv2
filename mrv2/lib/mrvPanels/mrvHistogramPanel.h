// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvPanelWidget.h"

#include "mrvFl/mrvColorAreaInfo.h"

class ViewerUI;

namespace mrv
{
    namespace area
    {
        class Info;
    }

    class HistogramPanel : public PanelWidget
    {
    public:
        HistogramPanel(ViewerUI* ui);
        ~HistogramPanel();

        void add_controls() override;

        void update(const area::Info& info);

    private:
        MRV2_PRIVATE();
    };

} // namespace mrv
