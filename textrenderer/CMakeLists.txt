# CMakeList.txt : CMake project for textrenderer, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (textrenderer "textrenderer.cpp" "textrenderer.h" "unicode_ranges.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET textrenderer PROPERTY CXX_STANDARD 20)
endif()

find_package(Fontconfig REQUIRED)
find_package(Freetype REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(cairo REQUIRED IMPORTED_TARGET cairo)
find_library(PANGO_LIBRARY pango-1.0)
find_library(PANGOCAIRO_LIBRARY pangocairo-1.0)
find_package(harfbuzz CONFIG REQUIRED)

target_include_directories(textrenderer PRIVATE 
  "${CMAKE_INCLUDE_PATH}/pango-1.0/"
  "${CMAKE_INCLUDE_PATH}/glib-2.0"
  "${CMAKE_PREFIX_PATH}/lib/glib-2.0/include"
)
target_link_libraries(textrenderer 
  Fontconfig::Fontconfig
  Freetype::Freetype
  harfbuzz::harfbuzz
  PkgConfig::cairo
  ${PANGO_LIBRARY}
  ${PANGOCAIRO_LIBRARY}
)

# TODO: Add tests and install targets if needed.
