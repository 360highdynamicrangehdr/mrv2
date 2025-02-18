// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlTimeline/Audio.h>
#include <tlTimeline/ReadCache.h>
#include <tlTimeline/Video.h>

#include <tlCore/Context.h>

#include <opentimelineio/composable.h>
#include <opentimelineio/item.h>
#include <opentimelineio/timeline.h>

#include "mrvApp/mrvFilesModel.h"

namespace tl
{
    namespace timeline
    {

        otio::SerializableObject::Retainer<otio::Timeline> create(
            const std::vector<std::shared_ptr<mrv::FilesModelItem> >& fileItems,
            const std::shared_ptr<system::Context>& context,
            const bool relative = false, const std::string& otioFilename = "",
            const Options& options = Options(),
            const std::shared_ptr<ReadCache>& readCache = nullptr);

    }

} // namespace tl
