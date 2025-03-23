#pragma once

#include "Math/float2.h"
#include "Math/float3.h"
#include "Math/float4.h"
#include "glew.h"
#include <map>

// Reference tutorial: https://nehe.gamedev.net/tutorial/freetype_fonts_in_opengl/24001/

namespace TextManager
{
    struct FontData
    {
        float fontHeight;
        // GLuint* textures;
        // GLuint listBase;
        GLuint textureId = 0;
        std::map<char, float4> charTextureCoords;

        void Init(const char* filename, const unsigned int height);
        void Clean() const;
    };

    // void RenderText(const FontData& font, const float2& position, const char* text);
    void RenderText(FontData& fontData, const std::string& text, const float3& position, const float3& scale);
} // namespace TextManager
