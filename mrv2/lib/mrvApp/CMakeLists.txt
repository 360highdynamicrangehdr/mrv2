# SPDX-License-Identifier: BSD-3-Clause
# mrv2 
# Copyright Contributors to the mrv2 Project. All rights reserved.

set(HEADERS
    App.h
    mrvDevicesModel.h
    mrvFilesModel.h
    mrvMainControl.h
    mrvOpenSeparateAudioDialog.h
    mrvPlaylistsModel.h
    mrvSettingsObject.h
    mrvUSD.h
  )
  set(SOURCES
    App.cpp
    mrvDevicesModel.cpp
    mrvFilesModel.cpp
    mrvMainControl.cpp
    mrvOpenSeparateAudioDialog.cpp
    mrvPlaylistsModel.cpp
    mrvSettingsObject.cpp
    mrvUSD.cpp
  )

add_library(mrvApp ${SOURCES} ${HEADERS})
files_to_absolute_paths()

set( GL_LIBRARY )
if (TLRENDER_GL)
    set( GL_LIBRARY mrvGL )
endif()

target_link_libraries(mrvApp PUBLIC mrvPanels ${GL_LIBRARY} mrvFLU mrvWidgets tlApp )
target_include_directories(mrvApp PUBLIC ${INCLUDES} )
set_target_properties(mrvApp PROPERTIES FOLDER lib)
set_target_properties(mrvApp PROPERTIES PUBLIC_HEADER "${HEADERS}")



install(TARGETS mrvApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/mrv2/mrvApp
    COMPONENT libraries )
