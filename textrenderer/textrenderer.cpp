// textrenderer.cpp : Defines the entry point for the application.
//

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

#include <cairo/cairo-ft.h>

const std::string fontName = "Verdana";
const int fontSize = 48;
const int fontSlant = FC_SLANT_ITALIC;
const int fontWeight = FC_WEIGHT_DEMIBOLD;
const int fontWidth = FC_WIDTH_SEMIEXPANDED;

std::u8string u8text = 
u8"Image size adjusted to glyphrun width\n"
u8"System font retrieved with FONTCONFIG\n"
u8"Glyphruns are rendered with cairographics\n"
u8"Cyr: Привет мир! Mymr: မင်္ဂလာပါကမ္ဘာလောက\n"
u8"Arab: مرحبا بالعالم even low-level text API\n"
u8"of cairographics cannot do right-to-left\n"
u8"scripts: we need pangocairo ";
int main()
{
    hb_buffer_t* buf0 = hb_buffer_create();
    hb_unicode_funcs_t* unicode = hb_buffer_get_unicode_funcs(buf0);

    double textRunPos = 0;
    double textWidth = 0;
    double linePos = 0;

    FT_Library    library;

    int ft_error = FT_Init_FreeType(&library);
    FT_Face face{};
    double lineHeight = 1.33 * fontSize;

    std::vector<std::u8string> textRuns;
    TextAnalyzer(u8text, unicode, textRuns);

    hb_font_t* hb_font = nullptr;
    cairo_font_face_t* crFontFace = nullptr;

    cairo_surface_t* recorder = cairo_recording_surface_create(CAIRO_CONTENT_COLOR_ALPHA, nullptr);
    cairo_t* crrec = cairo_create(recorder);

    for (std::u8string_view textRun : textRuns)
    {
        if (textRun == u8"\n")
        {
            linePos += lineHeight;
            textWidth = std::max(textRunPos, textWidth);
            textRunPos = 0;
            continue;
        }
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

        FcFontSet* fs = FcFontSetCreate();

        FcResult result;
        FcPatternAddBool(pat, FC_SCALABLE, FcTrue);
        char bufScript[5];
        hb_tag_to_string(hb_script_to_iso15924_tag(hb_buffer_get_script(buf)), bufScript);
        bufScript[4] = 0;
        FcLangSet* langSet = FcLangSetCreate();
        FcLangSetAdd(langSet, (const FcChar8*)bufScript);
        FcLangResult langresult = FcLangSetHasLang(langSet, (const FcChar8*)"Mymr");
        FcPatternAddLangSet(pat, FC_LANG, langSet);
        if (strcmp(bufScript, "Mymr") == 0)
        {
            pat = FcNameParse((FcChar8*)(FcChar8*)"Myanmar Text");
            FcPatternAddInteger(pat, FC_WEIGHT, fontWeight);
        }
        else if (strcmp(bufScript, "Arab") == 0)
        {
            pat = FcNameParse((FcChar8*)(FcChar8*)"Arial");
            FcPatternAddInteger(pat, FC_WEIGHT, fontWeight);
        }
        else
        {
            FcPatternAddString(pat, FC_FAMILY, (FcChar8*)fontName.c_str());
            FcPatternAddInteger(pat, FC_SLANT, fontSlant);
            FcPatternAddInteger(pat, FC_WEIGHT, fontWeight);
            FcPatternAddInteger(pat, FC_WIDTH, fontWidth);
        }
        FcConfigSubstitute(0, pat, FcMatchPattern);
        FcDefaultSubstitute(pat);

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
        /*hb_font = hb_ft_font_create(face, 0);
        // https://stackoverflow.com/questions/75521682/how-to-get-unicode-codepoint-in-harfbuzz-after-calling-hb-shape
        // https://stackoverflow.com/questions/61653970/how-can-i-activate-subpixel-positioning-with-pango-and-pycairo
        // https://stackoverflow.com/questions/36697999/font-layouting-rendering-with-cairo-and-freetype
        unsigned int glyph_count = 0;
        hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buf, &glyph_count);
        hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);
        hb_shape(hb_font, buf, nullptr, 0);*/
        crFontFace = cairo_ft_font_face_create_for_ft_face(face, 0);
        cairo_set_font_face(crrec, crFontFace);
        cairo_set_font_size(crrec, fontSize);
        auto scaled_face = cairo_get_scaled_font(crrec);
        cairo_text_cluster_t* clusters = nullptr;
        int cluster_count;
        cairo_glyph_t* crglyphs = nullptr;
        int crglyph_count;
        cairo_text_cluster_flags_t cluster_flags;
        auto stat = cairo_scaled_font_text_to_glyphs(scaled_face, textRunPos, linePos,
            (const char*)textRun.data(), textRun.size(), &crglyphs, &crglyph_count,
            &clusters, &cluster_count, &cluster_flags);
        if (stat != CAIRO_STATUS_SUCCESS)
        {
            std::cout << "cannot get glyphs for text run " << (const char*)textRun.data() << std::endl;
            continue;
        }

        cairo_text_extents_t text_extents;
        cairo_scaled_font_glyph_extents(scaled_face, crglyphs, crglyph_count, &text_extents);
        textRunPos += text_extents.x_advance;
        int crglyph_ix = 0;
        for (unsigned int clix = 0; clix < cluster_count; ++clix)
        {
            cairo_text_cluster_t* cluster = &clusters[clix];
            cairo_glyph_t* crglyphs_in_cluster = &crglyphs[crglyph_ix];
            cairo_glyph_path(crrec, crglyphs_in_cluster, cluster->num_glyphs);
            
            cairo_set_source_rgba(crrec, 0.57, 0.33, 0.82, 1.0);
            cairo_fill_preserve(crrec);

            cairo_set_line_width(crrec, 2.5);
            cairo_set_source_rgba(crrec, 0.31, 0.73, 0.42, 2.0 / 3);
            cairo_stroke(crrec);

            crglyph_ix += cluster->num_glyphs;
        }
        /*double xadv = 0;
        for (unsigned int glix = 0; glix < glyph_count; ++glix)
        {
            cairo_glyph_t crglyph;
            crglyph.index = glyph_info[glix].codepoint;
            crglyph.x = glyph_pos[glix].x_offset + xadv + textRunPos;
            xadv += glyph_pos[glix].x_advance / 64.0;
            crglyph.y = linePos + glyph_pos[glix].y_offset;
            //crglyphs.emplace_back(crglyph);
        }
        textRunPos += xadv;*/

        hb_buffer_destroy(buf);
    }

    // first time the layout is done without drawing, only to compute the canvass size
    // then I do it again and draw the glyphs
    // TODO: reuse the operations to the utmost

    int width = std::max(textWidth, textRunPos), height = linePos + lineHeight;

    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t* cr = cairo_create(surface);
    cairo_matrix_t matrix{ 1, 0, 0, 1, 0, 3.0 * lineHeight / 4 };
    cairo_transform(cr, &matrix);
    cairo_set_source_surface(cr, recorder, 0.0, 0.0);
    cairo_paint(cr);
    FT_Done_Face(face);
    FT_Done_FreeType(library);

    cairo_surface_write_to_png(surface, "glyphrun.png");

    //delete[] crglyphs;

    cairo_destroy(crrec);
    cairo_surface_destroy(recorder);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);
}
