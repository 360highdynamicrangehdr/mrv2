// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvNetwork/mrvTCP.h"

#include "mrViewer.h"

namespace mrv
{

    ColorPanel* colorPanel = nullptr;
    FilesPanel* filesPanel = nullptr;
    ComparePanel* comparePanel = nullptr;
    PlaylistPanel* playlistPanel = nullptr;
    SettingsPanel* settingsPanel = nullptr;
    EnvironmentMapPanel* environmentMapPanel = nullptr;
    LogsPanel* logsPanel = nullptr;
    DevicesPanel* devicesPanel = nullptr;
    ColorAreaPanel* colorAreaPanel = nullptr;
    AnnotationsPanel* annotationsPanel = nullptr;
    ImageInfoPanel* imageInfoPanel = nullptr;
    HistogramPanel* histogramPanel = nullptr;
    VectorscopePanel* vectorscopePanel = nullptr;
    Stereo3DPanel* stereo3DPanel = nullptr;
    PythonPanel* pythonPanel = nullptr;
#ifdef MRV2_NETWORK
    NetworkPanel* networkPanel = nullptr;
#endif
#ifdef TLRENDER_USD
    USDPanel* usdPanel = nullptr;
#endif

    bool one_panel_only = false;

    void onePanelOnly(bool t)
    {
        one_panel_only = t;
        bool send = App::ui->uiPrefs->SendUI->value();
        if (send)
            tcp->pushMessage("One Panel Only", t);
    }

    bool onePanelOnly()
    {
        return one_panel_only;
    }

    void redrawPanelThumbnails()
    {
        if (filesPanel)
            filesPanel->redraw();
        if (comparePanel)
            comparePanel->redraw();
        if (stereo3DPanel)
            stereo3DPanel->redraw();
        bool send = App::ui->uiPrefs->SendTimeline->value();
        if (send)
            tcp->pushMessage("Redraw Panel Thumbnails", 0);
    }

    void refreshPanelThumbnails()
    {
        if (filesPanel)
            filesPanel->refresh();
        if (comparePanel)
            comparePanel->refresh();
        if (stereo3DPanel)
            stereo3DPanel->refresh();
    }

    void removePanels(ViewerUI* ui)
    {
        if (colorPanel && colorPanel->is_panel())
            color_panel_cb(nullptr, ui);
        if (filesPanel && filesPanel->is_panel())
            files_panel_cb(nullptr, ui);
        if (comparePanel && comparePanel->is_panel())
            compare_panel_cb(nullptr, ui);
        if (playlistPanel && playlistPanel->is_panel())
            playlist_panel_cb(nullptr, ui);
        if (settingsPanel && settingsPanel->is_panel())
            settings_panel_cb(nullptr, ui);
        if (logsPanel && logsPanel->is_panel())
            logs_panel_cb(nullptr, ui);
        if (pythonPanel && pythonPanel->is_panel())
            python_panel_cb(nullptr, ui);
        if (devicesPanel && devicesPanel->is_panel())
            devices_panel_cb(nullptr, ui);
        if (colorAreaPanel && colorAreaPanel->is_panel())
            color_area_panel_cb(nullptr, ui);
        if (annotationsPanel && annotationsPanel->is_panel())
            annotations_panel_cb(nullptr, ui);
        if (imageInfoPanel && imageInfoPanel->is_panel())
            image_info_panel_cb(nullptr, ui);
        if (histogramPanel && histogramPanel->is_panel())
            histogram_panel_cb(nullptr, ui);
        if (vectorscopePanel && vectorscopePanel->is_panel())
            vectorscope_panel_cb(nullptr, ui);
        if (stereo3DPanel && stereo3DPanel->is_panel())
            stereo3D_panel_cb(nullptr, ui);
#ifdef MRV2_NETWORK
        if (networkPanel && networkPanel->is_panel())
            network_panel_cb(nullptr, ui);
#endif
#ifdef TLRENDER_USD
        if (usdPanel && usdPanel->is_panel())
            usd_panel_cb(nullptr, ui);
#endif
    }

    void removeWindows(ViewerUI* ui)
    {
        if (colorPanel && !colorPanel->is_panel())
            color_panel_cb(nullptr, ui);
        if (filesPanel && !filesPanel->is_panel())
            files_panel_cb(nullptr, ui);
        if (comparePanel && !comparePanel->is_panel())
            compare_panel_cb(nullptr, ui);
        if (playlistPanel && !playlistPanel->is_panel())
            playlist_panel_cb(nullptr, ui);
        if (settingsPanel && !settingsPanel->is_panel())
            settings_panel_cb(nullptr, ui);
        if (logsPanel && !logsPanel->is_panel())
            logs_panel_cb(nullptr, ui);
        if (pythonPanel && !pythonPanel->is_panel())
            python_panel_cb(nullptr, ui);
        if (devicesPanel && !devicesPanel->is_panel())
            devices_panel_cb(nullptr, ui);
        if (colorAreaPanel && !colorAreaPanel->is_panel())
            color_area_panel_cb(nullptr, ui);
        if (annotationsPanel && !annotationsPanel->is_panel())
            annotations_panel_cb(nullptr, ui);
        if (imageInfoPanel && !imageInfoPanel->is_panel())
            image_info_panel_cb(nullptr, ui);
        if (histogramPanel && !histogramPanel->is_panel())
            histogram_panel_cb(nullptr, ui);
        if (vectorscopePanel && !vectorscopePanel->is_panel())
            vectorscope_panel_cb(nullptr, ui);
        if (stereo3DPanel && !stereo3DPanel->is_panel())
            stereo3D_panel_cb(nullptr, ui);
#ifdef MRV2_NETWORK
        if (networkPanel && !networkPanel->is_panel())
            network_panel_cb(nullptr, ui);
#endif
#ifdef TLRENDER_USD
        if (usdPanel && !usdPanel->is_panel())
            usd_panel_cb(nullptr, ui);
#endif
    }

    void color_panel_cb(Fl_Widget* w, ViewerUI* ui)
    {
        bool send = ui->uiPrefs->SendUI->value();
        if (send)
        {
            tcp->pushMessage("Color Panel", static_cast<bool>(!colorPanel));
        }

        if (colorPanel)
        {
            delete colorPanel;
            colorPanel = nullptr;
            return;
        }
        colorPanel = new ColorPanel(ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void files_panel_cb(Fl_Widget* w, ViewerUI* ui)
    {
        bool send = ui->uiPrefs->SendUI->value();
        if (send)
        {
            tcp->pushMessage("Files Panel", static_cast<bool>(!filesPanel));
        }

        if (filesPanel)
        {
            delete filesPanel;
            filesPanel = nullptr;
            return;
        }
        filesPanel = new FilesPanel(ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void compare_panel_cb(Fl_Widget* w, ViewerUI* ui)
    {
        bool send = ui->uiPrefs->SendUI->value();
        if (send)
        {
            tcp->pushMessage("Compare Panel", static_cast<bool>(!comparePanel));
        }

        if (comparePanel)
        {
            delete comparePanel;
            comparePanel = nullptr;
            return;
        }
        comparePanel = new ComparePanel(ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void playlist_panel_cb(Fl_Widget* w, ViewerUI* ui)
    {
        bool send = ui->uiPrefs->SendUI->value();
        if (send)
        {
            tcp->pushMessage(
                "Playlist Panel", static_cast<bool>(!playlistPanel));
        }

        if (playlistPanel)
        {
            delete playlistPanel;
            playlistPanel = nullptr;
            return;
        }
        playlistPanel = new PlaylistPanel(ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void settings_panel_cb(Fl_Widget* w, ViewerUI* ui)
    {
        bool send = ui->uiPrefs->SendUI->value();
        if (send)
        {
            tcp->pushMessage(
                "Settings Panel", static_cast<bool>(!settingsPanel));
        }

        if (settingsPanel)
        {
            delete settingsPanel;
            settingsPanel = nullptr;
            return;
        }
        settingsPanel = new SettingsPanel(ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void python_panel_cb(Fl_Widget* w, ViewerUI* ui)
    {
        bool send = ui->uiPrefs->SendUI->value();
        if (send)
        {
            tcp->pushMessage("Python Panel", static_cast<bool>(!pythonPanel));
        }

        if (pythonPanel)
        {
            delete pythonPanel;
            pythonPanel = nullptr;
            return;
        }
        pythonPanel = new PythonPanel(ui);
#ifdef __APPLE__
        if (!ui->uiPrefs->uiPrefsMacOSMenus->value())
#endif
            ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void logs_panel_cb(Fl_Widget* w, ViewerUI* ui)
    {
        // Logs panel is not sent nor received.
        if (logsPanel)
        {
            delete logsPanel;
            logsPanel = nullptr;
            return;
        }
        logsPanel = new LogsPanel(ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void devices_panel_cb(Fl_Widget* w, ViewerUI* ui)
    {
        bool send = ui->uiPrefs->SendUI->value();
        if (send)
        {
            tcp->pushMessage("Devices Panel", static_cast<bool>(!devicesPanel));
        }

        if (devicesPanel)
        {
            delete devicesPanel;
            devicesPanel = nullptr;
            return;
        }
        devicesPanel = new DevicesPanel(ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void color_area_panel_cb(Fl_Widget* w, ViewerUI* ui)
    {
        bool send = ui->uiPrefs->SendUI->value();
        if (send)
        {
            tcp->pushMessage(
                "Color Area Panel", static_cast<bool>(!colorAreaPanel));
        }

        if (colorAreaPanel)
        {
            delete colorAreaPanel;
            colorAreaPanel = nullptr;
            return;
        }
        colorAreaPanel = new ColorAreaPanel(ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void annotations_panel_cb(Fl_Widget* w, ViewerUI* ui)
    {
        bool send = ui->uiPrefs->SendUI->value();
        if (send)
        {
            tcp->pushMessage(
                "Annotations Panel", static_cast<bool>(!annotationsPanel));
        }

        if (annotationsPanel)
        {
            delete annotationsPanel;
            annotationsPanel = nullptr;
            return;
        }
        annotationsPanel = new AnnotationsPanel(ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void image_info_panel_cb(Fl_Widget* w, ViewerUI* ui)
    {
        bool send = ui->uiPrefs->SendUI->value();
        if (send)
        {
            tcp->pushMessage(
                "Media Info Panel", static_cast<bool>(!imageInfoPanel));
        }

        if (imageInfoPanel)
        {
            delete imageInfoPanel;
            imageInfoPanel = nullptr;
            return;
        }
        imageInfoPanel = new ImageInfoPanel(ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void histogram_panel_cb(Fl_Widget* w, ViewerUI* ui)
    {
        bool send = ui->uiPrefs->SendUI->value();
        if (send)
        {
            tcp->pushMessage(
                "Histogram Panel", static_cast<bool>(!histogramPanel));
        }

        if (histogramPanel)
        {
            delete histogramPanel;
            histogramPanel = nullptr;
            return;
        }
        histogramPanel = new HistogramPanel(ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void vectorscope_panel_cb(Fl_Widget* w, ViewerUI* ui)
    {
        bool send = ui->uiPrefs->SendUI->value();
        if (send)
        {
            tcp->pushMessage(
                "Vectorscope Panel", static_cast<bool>(!vectorscopePanel));
        }

        if (vectorscopePanel)
        {
            delete vectorscopePanel;
            vectorscopePanel = nullptr;
            return;
        }
        vectorscopePanel = new VectorscopePanel(ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void environment_map_panel_cb(Fl_Widget* w, ViewerUI* ui)
    {
        bool send = ui->uiPrefs->SendUI->value();
        if (send)
        {
            tcp->pushMessage(
                "Environment Map Panel",
                static_cast<bool>(!environmentMapPanel));
        }
        if (environmentMapPanel)
        {
            delete environmentMapPanel;
            environmentMapPanel = nullptr;
            return;
        }
        environmentMapPanel = new EnvironmentMapPanel(ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void network_panel_cb(Fl_Widget* w, ViewerUI* ui)
    {
#ifdef MRV2_NETWORK
        bool send = ui->uiPrefs->SendUI->value();
        if (send)
        {
            tcp->pushMessage("Network Panel", static_cast<bool>(!networkPanel));
        }
        if (networkPanel)
        {
            delete networkPanel;
            networkPanel = nullptr;
            return;
        }
        networkPanel = new NetworkPanel(ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
#endif
    }

    void usd_panel_cb(Fl_Widget* w, ViewerUI* ui)
    {
#ifdef TLRENDER_USD
        bool send = ui->uiPrefs->SendUI->value();
        if (send)
        {
            tcp->pushMessage("USD Panel", static_cast<bool>(!networkPanel));
        }
        if (usdPanel)
        {
            delete usdPanel;
            usdPanel = nullptr;
            return;
        }
        usdPanel = new USDPanel(ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
#endif
    }

    void stereo3D_panel_cb(Fl_Widget* w, ViewerUI* ui)
    {
        bool send = ui->uiPrefs->SendUI->value();
        if (send)
        {
            tcp->pushMessage(
                "Stereo 3D Panel", static_cast<bool>(!stereo3DPanel));
        }
        if (stereo3DPanel)
        {
            delete stereo3DPanel;
            stereo3DPanel = nullptr;
            return;
        }
        stereo3DPanel = new Stereo3DPanel(ui);
        ui->uiMain->fill_menu(ui->uiMenuBar);
    }

    void syncPanels()
    {
        bool send = App::ui->uiPrefs->SendUI->value();
        if (send)
        {
            onePanelOnly(onePanelOnly()); // send one panel only setting first
            tcp->pushMessage("Files Panel", static_cast<bool>(filesPanel));
            tcp->pushMessage("Color Panel", static_cast<bool>(colorPanel));
            tcp->pushMessage("Compare Panel", static_cast<bool>(comparePanel));
            tcp->pushMessage(
                "Color Area Panel", static_cast<bool>(colorAreaPanel));
            tcp->pushMessage(
                "Playlist Panel", static_cast<bool>(playlistPanel));
            tcp->pushMessage(
                "Media Info Panel", static_cast<bool>(imageInfoPanel));
            tcp->pushMessage(
                "Annotations Panel", static_cast<bool>(annotationsPanel));
#ifdef TLRENDER_BMD
            tcp->pushMessage("Devices Panel", static_cast<bool>(devicesPanel));
#endif
            tcp->pushMessage(
                "Environment Map Panel",
                static_cast<bool>(environmentMapPanel));
            tcp->pushMessage(
                "Stereo 3D Panel", static_cast<bool>(stereo3DPanel));
            tcp->pushMessage(
                "Settings Panel", static_cast<bool>(settingsPanel));
            tcp->pushMessage("Python Panel", static_cast<bool>(pythonPanel));
#ifdef MRV2_NETWORK
            tcp->pushMessage("Network Panel", static_cast<bool>(networkPanel));
#endif
            tcp->pushMessage(
                "Histogram Panel", static_cast<bool>(histogramPanel));
            tcp->pushMessage(
                "Vectorscope Panel", static_cast<bool>(vectorscopePanel));
        }
    }
} // namespace mrv
