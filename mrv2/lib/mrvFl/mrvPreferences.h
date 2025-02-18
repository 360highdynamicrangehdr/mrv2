// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#ifdef _WIN32
#    pragma warning(disable : 4275)
#endif

#include <string>

#include "mrvFl/mrvColorSchemes.h"

// OpenColorIO
#include <OpenColorIO/OpenColorIO.h>
namespace OCIO = OCIO_NAMESPACE;

class ViewerUI;
class PreferencesUI;

namespace mrv
{
    class SettingsObject;

    class Preferences
    {
    public:
        Preferences(PreferencesUI* uiPrefs, bool reset);
        ~Preferences();

        static void run(ViewerUI* ui);
        static void save();

        static void open_windows();

        static void OCIO(ViewerUI* ui);

        static void updateICS();

        static OCIO::ConstConfigRcPtr OCIOConfig() { return config; }

    protected:
        static bool set_transforms();

    public:
        static ColorSchemes schemes;
        static bool native_file_chooser;
        static int bgcolor;
        static int textcolor;
        static int selectioncolor;
        static int selectiontextcolor;
        static int switching_images;

        static OCIO::ConstConfigRcPtr config;
        static std::string OCIO_Display;
        static std::string OCIO_View;

        static std::string root;
        static std::string hotkeys_file;
        static int language_index;
        static int debug;
    };

} // namespace mrv
