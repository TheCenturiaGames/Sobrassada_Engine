#pragma once

#include "Math/float2.h"
#include <map>


// Reference tutorial: https://nehe.gamedev.net/tutorial/freetype_fonts_in_opengl/24001/

namespace TextManager
{
    struct Character
    {
        unsigned int textureID = 0;
        float2 size            = float2::zero;
        float2 bearing         = float2::zero;
        unsigned int advance   = 0;
    };

    struct FontData
    {
        unsigned int fontSize;
        std::map<char, Character> characters;

        void Init(const char* filename, const unsigned int fontSize);
        void Clean();
    };

    void RenderText(FontData& fontData, const std::string& text, const unsigned vbo, const float maxWidth);

} // namespace TextManager
