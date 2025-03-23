#include "TextManager.h"
#include "Globals.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace TextManager
{
    int NextPower2(int numToSurpass)
    {
        int power = 1;
        while (power < numToSurpass)
        {
            power *= 2;
        }
        return power; // Returns the first power of 2 bigger than the given number
    }

    void MakeDisplayList(FT_Face font, char ch, GLuint listBase, GLuint* textureBase)
    {
        // Get the character bitmap
        bool error = FT_Load_Glyph(font, FT_Get_Char_Index(font, ch), FT_LOAD_DEFAULT);
        if (error)
        {
            GLOG("Error in loading glyph");
            return;
        }

        FT_GlyphSlot glyphSlot = font->glyph;
        error                  = FT_Render_Glyph(glyphSlot, FT_RENDER_MODE_NORMAL);
        if (error)
        {
            GLOG("Error in loading glyph");
            return;
        }
        const FT_Bitmap& bitmap = glyphSlot->bitmap;

        // Add padding to make it usable with an OpenGL texture
        const int width         = NextPower2(bitmap.width);
        const int height        = NextPower2(bitmap.rows);
        GLubyte* expandedData   = new GLubyte[2 * width * height];
        for (int j = 0; j < height; j++)
        {
            for (int i = 0; i < width; i++)
            {
                expandedData[2 * (i + j * width)] = 255;
                expandedData[2 * (i + j * width) + 1] =
                    (i >= bitmap.width || j >= bitmap.rows) ? 0 : bitmap.buffer[i + bitmap.width * j];
            }
        }

        // Bind the texture and set parameters
        glBindTexture(GL_TEXTURE_2D, textureBase[ch]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Create the texture
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_LUMINANCE_ALPHA, GL_UNSIGNED_BYTE, expandedData);
        delete expandedData;

        // Create a GL list with a series of commands that the texture of each character will need
        glNewList(listBase + ch, GL_COMPILE);
        glBindTexture(GL_TEXTURE_2D, textureBase[ch]);

        glPushMatrix();
        glTranslatef(glyphSlot->bitmap_left, 0, 0);
        glTranslatef(0, glyphSlot->bitmap_top - bitmap.rows, 0);

        const float x = (float)bitmap.width / (float)width;
        const float y = (float)bitmap.rows / (float)height;

        glBegin(GL_QUADS);
        glTexCoord2d(0, 0);
        glVertex2f(0, bitmap.rows);
        glTexCoord2d(0, y);
        glVertex2f(0, 0);
        glTexCoord2d(x, y);
        glVertex2f(bitmap.width, 0);
        glTexCoord2d(x, 0);
        glVertex2f(bitmap.width, bitmap.rows);
        glEnd();
        glPopMatrix();
        glTranslatef(glyphSlot->advance.x >> 6, 0, 0);

        glEndList();
    }

    void GenerateFontAtlas(const FT_Face font, GLuint& fontTexture, std::map<char, float4>& charTexCoords)
    {
        const int atlasWidth = 512, atlasHeight = 512;
        GLubyte* atlasData = new GLubyte[atlasWidth * atlasHeight];

        int xOffset = 0, yOffset = 0, rowHeight = 0;

        // 127 as of ASCII table
        for (char ch = 32; ch < 127; ++ch)
        {
            if (FT_Load_Char(font, ch, FT_LOAD_RENDER))
            {
                GLOG("ERROR: Failed to load character");
                continue;
            }
            if (FT_Render_Glyph(font->glyph, FT_RENDER_MODE_NORMAL))
            {
                GLOG("ERROR: Failed to load character");
                continue;
            }
            FT_Bitmap& bitmap = font->glyph->bitmap;

            if (xOffset + bitmap.width > atlasWidth)
            {
                xOffset    = 0;
                yOffset   += rowHeight;
                rowHeight  = 0;
            }

            for (int y = 0; y < bitmap.rows; y++)
            {
                for (int x = 0; x < bitmap.width; x++)
                {
                    atlasData[(yOffset + y) * atlasWidth + (xOffset + x)] = bitmap.buffer[y * bitmap.pitch + x];
                }
            }

            charTexCoords[ch] = float4(
                (float)xOffset / atlasWidth, (float)yOffset / atlasHeight, (float)bitmap.width / atlasWidth,
                (float)bitmap.rows / atlasHeight
            );

            xOffset   += bitmap.width + 2;
            rowHeight  = std::max(rowHeight, (int)bitmap.rows);

            glGenTextures(1, &fontTexture);
            glBindTexture(GL_TEXTURE_2D, fontTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlasWidth, atlasHeight, 0, GL_RED, GL_UNSIGNED_BYTE, atlasData);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        }
    }

    void FontData::Init(const char* filename, const unsigned int height)
    {
        // textures   = new GLuint[128];
        // fontHeight = height;

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
        FT_Set_Char_Size(face, height * 64, height * 64, 96, 96);

        // Generate and save the textures for each character
        // listBase = glGenLists(128);
        // glGenTextures(128, textures);
        // for (unsigned char i = 0; i < 128; i++)
        //{
        //    MakeDisplayList(face, i, listBase, textures);
        //}

        GenerateFontAtlas(face, textureId, charTextureCoords);

        FT_Done_Face(face);
        FT_Done_FreeType(library);
    }

    void FontData::Clean() const
    {
        if (textureId != 0) glDeleteTextures(1, &textureId);
        // glDeleteLists(listBase, 128);
        // glDeleteTextures(128, textures);
        // delete textures;
    }

    // void RenderText(const FontData& fontData, const float2& position, const char* text)
    //{
    //     GLuint fontList       = fontData.listBase;
    //     float height          = fontData.fontHeight / 0.63f; // Make the height bigger so we have spaces between
    //     lines
    //
    //     // In the tutorial here there is a part to get arguments from input text. For now it is not implemented, but
    //     it
    //     // may come in handy in the future
    //
    //     // Separate input text in different lines if there is a \n
    //     const char* startLine = text;
    //     std::vector<std::string> lines;
    //     const char* c = text;
    //     for (c; *c; c++)
    //     {
    //         if (*c == '\n')
    //         {
    //             std::string line;
    //             for (const char* n = startLine; n < c; n++)
    //             {
    //                 line.append(1, *n);
    //             }
    //             lines.push_back(line);
    //             startLine = c + 1;
    //         }
    //     }
    //     if (startLine)
    //     {
    //         std::string line;
    //         for (const char* n = startLine; n < c; n++)
    //         {
    //             line.append(1, *n);
    //         }
    //         lines.push_back(line);
    //     }
    //     delete c;
    //
    //     glPushAttrib(GL_LIST_BIT | GL_CURRENT_BIT | GL_ENABLE_BIT | GL_TRANSFORM_BIT);
    //     glMatrixMode(GL_MODELVIEW);
    //     glDisable(GL_LIGHTING);
    //     glEnable(GL_TEXTURE_2D);
    //     glDisable(GL_DEPTH_TEST);
    //     glEnable(GL_BLEND);
    //     glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //
    //     glListBase(fontList);
    // }

    void RenderText(FontData& fontData, const std::string& text, const float3& position, const float3& scale)
    {
        glBindTexture(GL_TEXTURE_2D, fontData.textureId);
        glEnable(GL_TEXTURE_2D);

        glPushMatrix();
        glTranslatef(position.x, position.y, position.z);
        glScalef(scale.x, scale.y, 1.0f);

        glBegin(GL_QUADS);
        float xOffset = 0.0f;

        for (char ch : text)
        {
            if (fontData.charTextureCoords.find(ch) == fontData.charTextureCoords.end()) continue;

            const float4& texCoord = fontData.charTextureCoords[ch];

            float w          = texCoord.z * 10.0f; // Scale width
            float h          = texCoord.w * 10.0f; // Scale height

            glTexCoord2f(texCoord.x, texCoord.y);
            glVertex3f(xOffset, 0, 0);

            glTexCoord2f(texCoord.x, texCoord.y + texCoord.w);
            glVertex3f(xOffset, h, 0);

            glTexCoord2f(texCoord.x + texCoord.z, texCoord.y + texCoord.w);
            glVertex3f(xOffset + w, h, 0);

            glTexCoord2f(texCoord.x + texCoord.z, texCoord.y);
            glVertex3f(xOffset + w, 0, 0);

            xOffset += w;
        }

        glEnd();
        glPopMatrix();
    }

} // namespace TextManager