# CMakeList.txt : CMake project for textrenderer, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (textrenderer "textrenderer.cpp" "textrenderer.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET textrenderer PROPERTY CXX_STANDARD 20)
endif()

if (MSVC)
  include("/vcpkg/scripts/buildsystems/vcpkg.cmake")
  target_include_directories(textrenderer PRIVATE 
                                            "C:/vcpkg/installed/x64-windows/include/"
                                            "C:/vcpkg/installed/x64-windows/include/fontconfig"
  											"C:/vcpkg/installed/x64-windows/include/pango-1.0"
											"C:/vcpkg/installed/x64-windows/include/glib-2.0"
											"C:/vcpkg/installed/x64-windows/lib/glib-2.0/include"
											"C:/vcpkg/installed/x64-windows/include/cairo"
  )
  find_library(FONTCONFIG_LIBRARY fontconfig)
  find_library(FREETYPE_LIBRARY freetype)
  find_library(CAIRO_LIBRARY cairo)
  find_library(PANGO_LIBRARY pango-1.0)
  find_library(PANGOCAIRO_LIBRARY pangocairo-1.0)

  find_package(harfbuzz CONFIG REQUIRED)

  target_link_libraries(textrenderer 
    ${FONTCONFIG_LIBRARY}
	${FREETYPE_LIBRARY}
	harfbuzz::harfbuzz
    ${CAIRO_LIBRARY}
    ${PANGO_LIBRARY}
    ${PANGOCAIRO_LIBRARY}
  )
else()
  target_include_directories(textrenderer PRIVATE 
                                            "/usr/include/fontconfig"
                                            "/usr/include/pango-1.0"
											"/usr/include/glib-2.0" 
											"/usr/lib/x86_64-linux-gnu/glib-2.0/include"
  											"/usr/include/freetype2"
											"/usr/include/cairo")
#find_package(fontconfig CONFIG REQUIRED)
  #target_link_libraries(textrenderer PRIVATE Fontconfig::Fontconfig)

  find_package(harfbuzz CONFIG REQUIRED)
  target_link_libraries(textrenderer PRIVATE
                                            harfbuzz::harfbuzz
                                            libfreetype.so
                                            libcairo.so
                                            libfontconfig.so)
endif()

# TODO: Add tests and install targets if needed.
