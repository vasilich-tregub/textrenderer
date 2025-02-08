// textrenderer.cpp : Defines the entry point for the application.
//

#define SHAPE_TEXTRUN

#include "textrenderer.h"
#include "unicode_ranges.h"
#include <vector>
#include <string>
#include <iostream>

#include <cairo/cairo.h>

#include <fontconfig/fontconfig.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_OUTLINE_H

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>

#ifndef HB_H_IN
#define HB_H_IN
#endif // !HB_H_IN
#include <harfbuzz/hb-unicode.h>
#include <harfbuzz/hb-cairo.h>

#include <cairo/cairo-ft.h>

std::u8string u8text = //u8"Arab:^الإصلاحي بزشكيان في طريقه_right-to-left script\n";
u8"FONTCONFIG is used to retrieve matching fonts, "
u8"harfbuzz splits the unicode text into text runs "
u8"according to textrun language writing system "
u8"and produces glyphruns from textruns. "
u8"Glyphruns are rendered with cairographics. \n"
u8"Cyr: Привет мир\n"
u8"Arab: مرحبابالعالم right-to-left script\n"
u8"Mymr: မင်္ဂလာပါကမ္ဘာလောက glyph clusters\n"
u8"(Implemented as a header only library.)";
int main()
{
    const std::string fontName = "Arial";
    const int fontSize = 24;
    const int fontWeight = FC_WEIGHT_REGULAR;
    const int fontSlant = FC_SLANT_ROMAN;
    const int fontWidth = FC_WIDTH_NORMAL;

    double margin = 12;
    double textWidth = 320;
    double lineHeight = fontSize;

    cairo_surface_t* recorder = cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA, nullptr);

    fontdesc fdesc{ fontName.c_str(), fontSize, fontWeight, fontSlant, fontWidth };

    double linePos = 0;

    RenderText(u8text, fdesc, recorder, lineHeight, textWidth, margin, &linePos);
    int width = textWidth + margin, height = linePos + lineHeight + margin;

    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t* cr = cairo_create(surface);
    cairo_matrix_t matrix{ 1, 0, 0, 1, 0, 3.0 * lineHeight / 4 };
    cairo_transform(cr, &matrix);
    cairo_set_source_surface(cr, recorder, 0.0, 0.0);
    cairo_paint(cr);

    auto status = cairo_surface_write_to_png(surface, "glyphrun.png");

    //delete[] crglyphs;

    cairo_surface_destroy(recorder);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}
