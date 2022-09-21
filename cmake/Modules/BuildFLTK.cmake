include( ExternalProject )

#set( FLTK_TAG 232743c3a5d903be813f6c4445f3f96bab25cae0 ) # this works but it is old
set( FLTK_TAG master )

set( patch_cmd ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/patches/FLTK/Fl_Window_fullscreen.cxx ${CMAKE_BINARY_DIR}/FLTK-prefix/src/FLTK/src )

if(APPLE)
  set( patch_cmd ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/patches/FLTK/fl_plastic.cxx ${CMAKE_CURRENT_SOURCE_DIR}/patches/FLTK/fl_gtk.cxx ${CMAKE_CURRENT_SOURCE_DIR}/patches/FLTK/fl_gleam.cxx ${CMAKE_CURRENT_SOURCE_DIR}/patches/FLTK/Fl_Window_fullscreen.cxx ${CMAKE_BINARY_DIR}/FLTK-prefix/src/FLTK/src && ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/patches/FLTK/CMakeLists.txt ${CMAKE_BINARY_DIR}/FLTK-prefix/src/FLTK/fluid )
else()
endif()

ExternalProject_Add(
  FLTK
  GIT_REPOSITORY "https://github.com/fltk/fltk.git"
  GIT_TAG ${FLTK_TAG}
  GIT_PROGRESS 1
  PATCH_COMMAND ${patch_cmd}
  CMAKE_ARGS
  -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
  -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
  -DFLTK_BUILD_EXAMPLES=OFF
  -DFLTK_BUILD_TEST=OFF
  -DOPTION_BUILD_SHARED_LIBS=0
  -DOPTION_USE_SYSTEM_ZLIB=0
  -DOPTION_USE_SYSTEM_LIBJPEG=0
  -DOPTION_USE_SYSTEM_LIBPNG=0
  -DOPTION_USE_WAYLAND=1
  ${INSTALL_CMD}
)
