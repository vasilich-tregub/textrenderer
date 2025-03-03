﻿# CMakeList.txt : CMake project for textrenderer, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (textrenderer-narrowcolumn "textrenderer-narrowcolumn.cpp" "textrenderer.h" "unicode_ranges.h")
add_executable (textrenderer-widecolumn "textrenderer-widecolumn.cpp" "textrenderer.h" "unicode_ranges.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET textrenderer-narrowcolumn PROPERTY CXX_STANDARD 20)
  set_property(TARGET textrenderer-widecolumn PROPERTY CXX_STANDARD 20)
endif()

find_package(Fontconfig REQUIRED)
find_package(Freetype REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(cairo REQUIRED IMPORTED_TARGET cairo)
find_package(harfbuzz CONFIG REQUIRED)

find_library(HB_CAIRO harfbuzz-cairo)

target_link_libraries(textrenderer-narrowcolumn PRIVATE
  Fontconfig::Fontconfig
  Freetype::Freetype
  harfbuzz::harfbuzz
  ${HB_CAIRO}
  PkgConfig::cairo
)

target_link_libraries(textrenderer-widecolumn PRIVATE
  Fontconfig::Fontconfig
  Freetype::Freetype
  harfbuzz::harfbuzz
  ${HB_CAIRO}
  PkgConfig::cairo
)
# TODO: Add tests and install targets if needed.
