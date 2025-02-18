# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set( FL_SOURCES
    make_ocio_chooser.fl
    keyboard_ui.fl
    mrvAboutUI.fl
    mrvHotkeyUI.fl
    mrvHUDUI.fl
    PathMappingUI.fl
    mrvPreferencesUI.fl
    SaveOptionsUI.fl
    mrViewer.fl
)

FLTK_WRAP_UI( mrvWidgets ${FL_SOURCES} )

set(HEADERS
    mrvButton.h
    mrvBrowser.h
    mrvChoice.h
    mrvClipButton.h
    mrvCollapsibleGroup.h
    mrvColorInfo.h
    mrvDockGroup.h
    mrvDragButton.h
    mrvDropWindow.h
    mrvFileButton.h
    mrvFileDragger.h
    mrvFunctional.h
    mrvHistogram.h
    mrvHorSlider.h
    mrvLogDisplay.h
    mrvMainWindow.h
    mrvMenuBar.h
    mrvMultilineInput.h
    mrvOCIOBrowser.h
    mrvPack.h
    mrvPathMappingBrowser.h
    mrvPanelButton.h
    mrvPanelGroup.h
    mrvPanelWindow.h
    mrvPopupAudio.h
    mrvPopupMenu.h
    mrvPreferencesTree.h
    mrvProgressReport.h
    mrvPythonEditor.h
    mrvPythonOutput.h
    mrvResizableBar.h
    mrvSecondaryWindow.h
    mrvSlider.h
    mrvStateButton.h
    mrvStatusBar.h
    mrvTable.h
    mrvTile.h
    mrvTextDisplay.h
    mrvTimecode.h
    mrvTimelineGroup.h
    mrvVectorscope.h
    mrvVersion.h
    mrvVolumeSlider.h
)

set(SOURCES
    ${mrvWidgets_FLTK_UI_SRCS}

    mrvBrowser.cpp
    mrvButton.cpp
    mrvChoice.cpp
    mrvCollapsibleGroup.cpp
    mrvColorInfo.cpp
    mrvDockGroup.cpp
    mrvDragButton.cpp
    mrvDropWindow.cpp
    mrvFileButton.cpp
    mrvFileDragger.cpp
    mrvHistogram.cpp
    mrvHorSlider.cpp
    mrvLogDisplay.cpp
    mrvMainWindow.cpp
    mrvMenuBar.cpp
    mrvMultilineInput.cpp
    mrvOCIOBrowser.cpp
    mrvPack.cpp
    mrvPathMappingBrowser.cpp
    mrvPanelButton.cpp
    mrvPanelGroup.cpp
    mrvPanelWindow.cpp
    mrvPopupAudio.cpp
    mrvPopupMenu.cpp
    mrvPreferencesTree.cpp
    mrvProgressReport.cpp
    mrvPythonEditor.cpp
    mrvPythonOutput.cpp
    mrvResizableBar.cpp
    mrvSecondaryWindow.cpp
    mrvSlider.cpp
    mrvStatusBar.cpp
    mrvTable.cpp
    mrvTextDisplay.cpp
    mrvTimecode.cpp
    mrvTimelineGroup.cpp
    mrvTile.cpp
    mrvVectorscope.cpp
    mrvVersion.cpp
    mrvVolumeSlider.cpp
)


files_to_absolute_paths()

if ( APPLE )
    list( PREPEND SOURCES mrvMainWindow.mm )
endif()

add_library(mrvWidgets ${SOURCES} ${HEADERS})

list( APPEND LIBRARIES mrvFlmm mrvFLU mrvCore fltk_images pybind11::embed )

if ( UNIX)
    if ( NOT APPLE )
	add_definitions( -DFLTK_USE_X11 )
	list(APPEND LIBRARIES Xss )
    endif()
endif()

target_link_libraries(mrvWidgets PUBLIC ${LIBRARIES} )
set_target_properties(mrvWidgets PROPERTIES FOLDER lib)
set_target_properties(mrvWidgets PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS mrvWidgets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv2/mrvWidgets
    COMPONENT libraries)
