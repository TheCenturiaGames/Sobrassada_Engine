#include "TextManager.h"

#include "Globals.h"
#include "FileSystem.h"
#include "Application.h"
#include "OpenGLModule.h"

#include "Math/float4x4.h"
#include "glew.h"
#include <ft2build.h>
#include FT_FREETYPE_H

namespace TextManager
{
    void GenerateFontTextures(const FT_Face face, std::map<char, Character>& outCharacters)
    {
        // Disable byte-alignment restriction.
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // 128 as of ASCII table
        for (unsigned char ch = 0; ch < 128; ch++)
        {
            if (FT_Load_Char(face, ch, FT_LOAD_RENDER))
            {
                GLOG("ERROR: Failed to load character");
                continue;
            }
            // if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL))
            //{
            //     GLOG("ERROR: Failed to load character");
            //     continue;
            // }

            GLuint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);

            glTexImage2D(
                GL_TEXTURE_2D, 0, GL_R8, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED,
                GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer
            );
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            UID bindlessUID = glGetTextureHandleARB(texture);
            glMakeTextureHandleResidentARB(bindlessUID);

            // Store the data of the character texture
            Character character = {
                texture, bindlessUID,
                float2(static_cast<float>(face->glyph->bitmap.width), static_cast<float>(face->glyph->bitmap.rows)),
                float2(static_cast<float>(face->glyph->bitmap_left), static_cast<float>(face->glyph->bitmap_top)),
                static_cast<unsigned int>(face->glyph->advance.x)
            };

            // Store the loaded glyph in the map for later use.
            outCharacters.insert(std::pair<char, Character>(ch, character));
        }

        // Reset pixel storage mode to default
        glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    }

    void FontData::Init(const char* filename, const unsigned int fontSize)
    {
        // Initialize the library
        FT_Library library;
        if (FT_Init_FreeType(&library)) GLOG("Error: Could not initialize FreeType");

        // Initialize the face of the selected font
        FT_Face face;
        if (FT_New_Face(library, filename, 0, &face))
        {
            GLOG("Error: Could not load the font");
        }

        // Set the font size
        FT_Set_Pixel_Sizes(face, 0, fontSize);

        // Generate a texture per character
        GenerateFontTextures(face, characters);
        this->fontSize = fontSize;
        this->fontName = face->family_name;

        FT_Done_Face(face);
        FT_Done_FreeType(library);
    }

    void FontData::Clean()
    {
        for (const auto& c : characters)
        {
            if (c.second.textureID != 0)
            {
                glMakeTextureHandleNonResidentARB(c.second.bindlessUID);
                glDeleteTextures(1, &c.second.textureID);
            }
            
        }
        characters.clear();
    }

    void RenderText(FontData& fontData, const std::string& text, const float3& startPos, const unsigned vbo, const float maxWidth)
    {
        // When deferred lightning works, this won't be needed as transparency will be properly handled
        glDisable(GL_DEPTH_TEST);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        int x = (int)startPos.x;
        int y = (int)startPos.y - fontData.fontSize;
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        for (char c : text)
        {
            if (c == '\n')
            {
                x  = 0;
                y -= fontData.fontSize;
                continue;
            }
            else if (x > maxWidth)
            {
                x  = 0;
                y -= fontData.fontSize;
            }

            const Character character = fontData.characters[c];

            const float xPos          = x + character.bearing.x;
            const float yPos          = y - (character.size.y - character.bearing.y);
            const float width         = character.size.x;
            const float height        = character.size.y;

            // Positions - Uvs interleaved
            const float vertices[]    = {xPos,         yPos + height, 0.0f,         0.0f, xPos, yPos,
                                         0.0f,         1.0f,          xPos + width, yPos, 1.0f, 1.0f,

                                         xPos + width, yPos + height, 1.0f,         0.0f, xPos, yPos + height,
                                         0.0f,         0.0f,          xPos + width, yPos, 1.0f, 1.0f};

            const GLuint lower = static_cast<GLuint>(character.bindlessUID & 0xFFFFFFFF);
            const GLuint higher = static_cast<GLuint>(character.bindlessUID >> 32);
            glUniform2ui(4, lower, higher);
            glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
            App->GetOpenGLModule()->DrawArrays(GL_TRIANGLES, 0, 6);

            x += (character.advance >> 6);
        }

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }

} // namespace TextManager