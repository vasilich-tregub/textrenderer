﻿// textrenderer.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vector>
#include <string>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#ifndef HB_H_IN
#define HB_H_IN
#endif // !HB_H_IN
#include <harfbuzz/hb-unicode.h>
#include <fontconfig/fontconfig.h>

struct fontdesc
{
    const char* fontName;
    const int fontSize;
    const int fontWeight;
    const int fontSlant;
    const int fontWidth;

};

int u8seqToChar32cp(std::u8string::iterator& iter, std::u8string::iterator iterEnd, char32_t* code_point)
{
    if (iter == iterEnd)
        return 0;

    std::u8string::iterator iter_saved = iter;

    uint32_t cp = 0;

    int seqLen = 0;
    if (*iter < 0x80)
        seqLen = 1;
    else if ((*iter >> 5) == 0x6)
        seqLen = 2;
    else if ((*iter >> 4) == 0xe)
        seqLen = 3;
    else if ((*iter >> 3) == 0x1e)
        seqLen = 4;

    // Get trail octets and calculate the code point
    int err = 0;
    switch (seqLen) {
    case 0:
        return -2; // illegal start of code unit
    case 1:
        cp = *iter;
        break;
    case 2:
        cp = *iter;
        if (iter++ == iterEnd)
        {
            iter = iter_saved;
            return -1; // unfinished code unit (unexpected u8string end)
        }
        cp = ((cp << 6) & 0x7ff) + ((*iter) & 0x3f);
        break;
    case 3:
        cp = *iter;
        if (iter++ == iterEnd)
        {
            iter = iter_saved;
            return -1;
        }
        cp = ((cp << 12) & 0xffff) + ((*iter << 6) & 0xfff);
        if (iter++ == iterEnd)
        {
            iter = iter_saved;
            return -1;
        }
        cp += (*iter) & 0x3f;
        break;
    case 4:
        cp = *iter;
        if (iter++ == iterEnd)
        {
            iter = iter_saved;
            return -1;
        }
        cp = ((cp << 18) & 0x1fffff) + ((*iter << 12) & 0x3ffff);
        if (iter++ == iterEnd)
        {
            iter = iter_saved;
            return -1;
        }
        cp += (*iter << 6) & 0xfff;
        if (iter++ == iterEnd)
        {
            iter = iter_saved;
            return -1;
        }
        cp += (*iter) & 0x3f;
        break;
    }

    // TODO: check if codepoint is valid
    /*if (cp.isValid) {*/
    *code_point = cp;
    ++iter;
    return 0;
    /*}
    * else
    * {
    *   iter = iter_saved
        return -3; //char32_t codepoint is invalid
    * }
    */
}

int TextAnalyzer(std::u8string& stringToAnalyze, hb_unicode_funcs_t* unicode, std::vector<std::u8string>& textRuns)
{
    int ret = 0;
    hb_script_t runScript = HB_SCRIPT_UNKNOWN;
    char32_t cp;
    auto iter = begin(stringToAnalyze);
    auto runStart = iter;
    auto runEnd = runStart;
    if ((ret = u8seqToChar32cp(iter, end(stringToAnalyze), &cp)) != 0)
        return ret;
    runScript = hb_unicode_script(unicode, cp);
    for (; iter != end(stringToAnalyze);)
    {
        runEnd = iter;
        if ((ret = u8seqToChar32cp(iter, end(stringToAnalyze), &cp)) != 0)
            return ret;
        hb_script_t script = hb_unicode_script(unicode, cp); // for hb's "arab:^طريقه_RTL", my logic renders "arab:^_طريقهRTL"
        if (script != runScript/* && script != HB_SCRIPT_COMMON*/) //  (both HB_SCRIPT_COMMON cp's, ^ and _, come before HB_SCRIPT_ARAB cp's)
        {
            textRuns.emplace_back(runStart, runEnd);
            runStart = runEnd;
            runScript = script;
        }
    }
    if (runStart != runEnd)
        textRuns.emplace_back(runStart, end(stringToAnalyze));
    return 0;
}

int setFont(const char* fontName = "Arial",
    const int fontSize = 14,
    const int fontWeight = FC_WEIGHT_REGULAR,
    const int fontSlant = FC_SLANT_ROMAN,
    const int fontWidth = FC_WIDTH_NORMAL
)
{
    return 0;
}

int ShapeTextRun(const char* text, FT_Library library, fontdesc fdesc, FT_Face* face, hb_buffer_t* buf)
{
    hb_buffer_add_utf8(buf, text, (int)strlen(text), 0, (int)strlen(text));

    hb_buffer_guess_segment_properties(buf);

    FcPattern* pat = FcPatternCreate();

    if (!pat)
    {
        //std::cout << "Cannot create the pattern.\n";
        return -1;
    }

    FcResult result;
    FcPatternAddBool(pat, FC_SCALABLE, FcTrue);
    char bufScript[5];
    hb_tag_to_string(hb_script_to_iso15924_tag(hb_buffer_get_script(buf)), bufScript);
    bufScript[4] = 0;
    FcLangSet* langSet = FcLangSetCreate();
    FcLangSetAdd(langSet, (const FcChar8*)bufScript);
    FcPatternAddLangSet(pat, FC_LANG, langSet);
    FcPatternAddString(pat, FC_FAMILY, (FcChar8*)fdesc.fontName);
    FcPatternAddInteger(pat, FC_WEIGHT, fdesc.fontWeight);
    FcPatternAddInteger(pat, FC_WIDTH, fdesc.fontWidth);
    if (strcmp(bufScript, "Mymr") == 0)
    {
        pat = FcNameParse((FcChar8*)"Myanmar Text");
        FcPatternAddInteger(pat, FC_WEIGHT, fdesc.fontWeight);
    }
    else if (strcmp(bufScript, "Arab") == 0)
    {
        // no italic for arabic font
    }
    else
    {
        FcPatternAddInteger(pat, FC_SLANT, fdesc.fontSlant);
    }
    FcConfigSubstitute(0, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);

    FcPattern* match = FcFontMatch(0, pat, &result);
    if (!match)
        /*FcFontSetAdd(fs, match);
    else*/
    {
        //std::cout << "No matching font string found.\n";
        return -1;
    }

    FcChar8* fontfile = nullptr;
    if (FcResultMatch != FcPatternGetString(match, FC_FILE, 0, &fontfile))
    {
        //std::cout << "No matching font string found.\n";
        return -1;
    }

    int ft_error;
    if ((ft_error = FT_New_Face(library, (char*)fontfile, 0, face)) != 0)
    {
        //std::cout << "FT_New_Face returns error code " << ft_error << "\n";
        return -1;
    }

    FcPatternDestroy(pat);

    ft_error = FT_Set_Pixel_Sizes(*face, 0, fdesc.fontSize);
    hb_font_t* hb_font = hb_ft_font_create(*face, 0);

    unsigned int glyph_count = 0;
    hb_shape(hb_font, buf, nullptr, 0);
    return 0;
}