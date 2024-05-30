// textrenderer.cpp : Defines the entry point for the application.
//

#include "textrenderer.h"
#include <vector>
#include <string>

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

#include <pango-1.0/pango/pangocairo.h>
#include <cairo/cairo-ft.h>

const std::string fontName = "Verdana";
const int fontSize = 48;
const int fontSlant = FC_SLANT_ITALIC;
const int fontWeight = FC_WEIGHT_DEMIBOLD;
const int fontWidth = FC_WIDTH_SEMIEXPANDED;

int main()
{
    int width = 832, height = 120;
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_A8, width, height);
    cairo_t* cr = cairo_create(surface);
    //cairo_set_tolerance(cr, 0.5);
    cairo_matrix_t matrix{ 1, 0, 0, 1, 0, 3.0 * height / 4 };
    cairo_transform(cr, &matrix);

    //hb_buffer_t* buf0 = hb_buffer_create();
    //hb_unicode_funcs_t* unicode = hb_buffer_get_unicode_funcs(buf0);

    double textRunPos = 0;

    FT_Library    library;

    int ft_error = FT_Init_FreeType(&library);
    FT_Face face;

    std::u8string textRun = u8"Font found with FONTCONFIG";

    const char* text = reinterpret_cast<const char*>(textRun.data());

    hb_buffer_t* buf = hb_buffer_create();

    hb_buffer_add_utf8(buf, text, (int)strlen(text), 0, (int)strlen(text));

    hb_buffer_guess_segment_properties(buf);

    FcPattern* pat = FcPatternCreate();

    if (!pat)
    {
        std::cout << "Cannot create the pattern.\n";
        return -1;
    }

    FcConfigSubstitute(0, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);

    FcFontSet* fs = FcFontSetCreate();

    FcResult result;
    pat = FcNameParse((FcChar8*)fontName.c_str());
    FcPatternAddInteger(pat, FC_SLANT, fontSlant);
    FcPatternAddInteger(pat, FC_WEIGHT, fontWeight);
    FcPatternAddInteger(pat, FC_WIDTH, fontWidth);

    FcPattern* match = FcFontMatch(0, pat, &result);
    if (!match)
        /*FcFontSetAdd(fs, match);
    else*/
    {
        std::cout << "No matching font string found.\n";
        return -1;
    }

    FcChar8* fontfile = nullptr;
    if (FcResultMatch != FcPatternGetString(match, FC_FILE, 0, &fontfile))
    {
        std::cout << "No matching font string found.\n";
        return -1;
    }

    if ((ft_error = FT_New_Face(library, (char*)fontfile, 0, &face)) != 0)
    {
        std::cout << "FT_New_Face returns error code " << ft_error << "\n";
        return -1;
    }

    FcPatternDestroy(pat);

    ft_error = FT_Set_Pixel_Sizes(face, 0, fontSize);
    hb_font_t* hb_font = hb_ft_font_create(face, 0);
    hb_shape(hb_font, buf, nullptr, 0);
    cairo_font_face_t* crFontFace = cairo_ft_font_face_create_for_ft_face(face, 0);
    cairo_set_font_face(cr, crFontFace);

    unsigned int glyph_count = 0;
    hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buf, &glyph_count);
    hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);
    cairo_set_font_size(cr, fontSize);

    cairo_glyph_t* crglyphs = new cairo_glyph_t[glyph_count];

    double xadv = 0;
    for (unsigned int glix = 0; glix < glyph_count; ++glix)
    {
        crglyphs[glix].index = glyph_info[glix].codepoint;
        crglyphs[glix].x = glyph_pos[glix].x_offset + xadv + textRunPos;
        xadv += glyph_pos[glix].x_advance / 64.0;
        crglyphs[glix].y = glyph_pos[glix].y_offset;
    }
    textRunPos += xadv;

    cairo_glyph_path(cr, crglyphs, glyph_count);

    hb_font_destroy(hb_font);

    hb_buffer_destroy(buf);

    FT_Done_Face(face);
    FT_Done_FreeType(library);

    // draw unaltered glyphrun
    cairo_fill(cr);
    cairo_surface_write_to_png(surface, "glyphrun.png");

    delete[] crglyphs;

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}
