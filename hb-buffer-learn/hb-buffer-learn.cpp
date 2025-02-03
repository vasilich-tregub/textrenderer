// textrenderer.cpp : Defines the entry point for the application.
//

#include "hb-buffer-learn.h"
#include <vector>
#include <string>
#include <iostream>

#include <harfbuzz/hb.h>
//#include <harfbuzz/hb-ft.h>

#include <string.h>

#ifndef HB_H_IN
#define HB_H_IN
#endif // !HB_H_IN
//#include <harfbuzz/hb-unicode.h>
//#include <harfbuzz/hb-cairo.h>

//std::u8string u8text = u8"Кириллик Latin script is LTR\n";
//std::u8string u8text = u8"Arab الإصلاحي بزشكيان في طريقه \n";
//std::u8string u8text = u8" الإصلاحي بزشكيان في طريقه Arab\n";
std::u8string u8text = u8"a1( في )z";

int main()
{
    const char* text = reinterpret_cast<const char*>(u8text.data());
    for (auto u8char : u8text)
    {
        std::cout << (unsigned int)u8char << std::endl;
    }
    std::cout << std::endl;

    hb_buffer_t* buf = hb_buffer_create();
    hb_unicode_funcs_t* ufuncs = hb_buffer_get_unicode_funcs(buf);

    hb_buffer_add_utf8(buf, text, (int)strlen(text), 0, (int)strlen(text));

    std::vector<std::u8string> textRuns{};
    TextAnalyzer(u8text, ufuncs, textRuns);

    for (auto textRun : textRuns)
    {
        const char* txt = reinterpret_cast<const char*>(textRun.data());
        std::cout << txt << std::endl;
    }
    std::cout << std::endl;

    hb_buffer_guess_segment_properties(buf);

    hb_script_t script = hb_buffer_get_script(buf);
    char bufScript[5];
    hb_tag_to_string(hb_script_to_iso15924_tag(script), bufScript);
    bufScript[4] = 0;

    std::cout << text << std::endl;
    std::cout << bufScript << std::endl;

    hb_direction_t direction = hb_script_get_horizontal_direction(script);
    const char* directionStr = hb_direction_to_string(direction);
    std::cout << "direction = " << directionStr << std::endl;
}
