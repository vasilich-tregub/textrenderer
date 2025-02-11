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
u8"(Function 'RenderText' is implemented in a header only library.)";
int main()
{
    const std::string fontName = "Arial";
    const int fontSize = 36;
    const int fontWeight = FC_WEIGHT_REGULAR;
    const int fontSlant = FC_SLANT_ROMAN;
    const int fontWidth = FC_WIDTH_NORMAL;

    double margin = 12;
    double textWidth = 480;
    double lineHeight = fontSize;

    cairo_surface_t* recorder = cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA, nullptr);

    fontdesc fdesc{ fontName.c_str(), fontSize, fontWeight, fontSlant, fontWidth };

    colorstruct fillColor{ 0.290, 0.392, 0.424, 1. }; // deep space sparkle
    colorstruct outlineColor{ 0.757, 0.329, 0.757, 1. }; // deep fuchsia

    double linePos = 0;

    RenderText(u8text, fdesc, fillColor, outlineColor, 1.5,
        recorder, lineHeight, textWidth, margin, &linePos);
    int width = textWidth + margin, height = linePos + lineHeight + margin;

    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t* cr = cairo_create(surface);
    // Set surface to color (r, g, b, a) (sort of text background here)
    cairo_save(cr);
    cairo_set_source_rgba(cr, 0.25, 0.25, 0.25, 0.125);
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_paint(cr);
    cairo_restore(cr);

    cairo_matrix_t matrix{ 1, 0, 0, 1, 0, 0 };
    cairo_transform(cr, &matrix);
    cairo_set_source_surface(cr, recorder, 0.0, 0.0);
    cairo_paint(cr);

    // get pixel data from image surface (only to demonstrate function 'cairo_image_surface_get_data')
    unsigned char* data = cairo_image_surface_get_data(surface);
    // write image surface to png
    auto status = cairo_surface_write_to_png(surface, "narrow_column.png");

    //delete[] crglyphs;

    cairo_surface_destroy(recorder);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}
