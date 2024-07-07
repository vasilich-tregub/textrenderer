// textrenderer.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <vector>
#include <string>
#ifndef HB_H_IN
#define HB_H_IN
#endif // !HB_H_IN
#include <harfbuzz/hb-unicode.h>

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
    std::vector<std::u8string> rtlTextRuns{};
    int ret = 0;
    hb_script_t runScript = HB_SCRIPT_UNKNOWN;
    hb_direction_t runDirection = HB_DIRECTION_LTR; // or HB_DIRECTION_INVALID? No, better start always with LTR direction
    char32_t cp;
    auto iter = begin(stringToAnalyze);
    auto runStart = iter;
    auto runEnd = runStart;
    if ((ret = u8seqToChar32cp(iter, end(stringToAnalyze), &cp)) != 0)
        return ret;
    runScript = hb_unicode_script(unicode, cp);
    hb_direction_t directionBefore = hb_script_get_horizontal_direction(runScript);
    auto insertIter = begin(textRuns);
    int rtlRunsCount = 0;
    for (; iter != end(stringToAnalyze);)
    {
        runEnd = iter;
        if ((ret = u8seqToChar32cp(iter, end(stringToAnalyze), &cp)) != 0)
            return ret;
        hb_script_t script = hb_unicode_script(unicode, cp);
        hb_direction_t direction = hb_script_get_horizontal_direction(script);
        if (script != runScript) // todo: process HB_SCRIPT_COMMON case to account for blank space, 0x20;
        {
            std::u8string textrun(runStart, runEnd);
            if (direction == HB_DIRECTION_LTR)
            {
                if (script != HB_SCRIPT_COMMON)
                {
                    rtlRunsCount = 0;
                }
                insertIter = textRuns.end();
            }
            else
            {
                if (directionBefore == HB_DIRECTION_LTR)
                {
                    insertIter = textRuns.end();
                    directionBefore = direction;
                    ++rtlRunsCount;
                    insertIter = end(textRuns) - rtlRunsCount;
                }
                else
                {
                    ++rtlRunsCount;
                    insertIter = end(textRuns) - rtlRunsCount;
                }
            }
            textRuns.insert(insertIter, textrun);
            runStart = runEnd;
            runScript = script;
        }
    }
    if (runStart != runEnd)
        textRuns.emplace_back(runStart, end(stringToAnalyze));
    return 0;
}