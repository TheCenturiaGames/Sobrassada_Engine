#include "TextureImporter.h"

#include "Application.h"
#include "DirectXTex/DirectXTex.h"
#include "FileSystem.h"
#include "LibraryModule.h"
#include "MetaTexture.h"
#include "glew.h"
#include <string>

namespace TextureImporter
{
    UID Import(const char* filePath)
    {
        // Copy image to Assets folder
        {
            std::string copyPath = ASSETS_PATH + FileSystem::GetFileNameWithExtension(filePath);
            if (!FileSystem::Exists(copyPath.c_str()))
            {
                FileSystem::Copy(filePath, copyPath.c_str());
            }
        }

        std::string textureStr = std::string(filePath);
        std::wstring wPath     = std::wstring(textureStr.begin(), textureStr.end());

        DirectX::ScratchImage image;
        HRESULT hr = DirectX::LoadFromWICFile(wPath.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, image);

        if (FAILED(hr))
        {
            hr = DirectX::LoadFromTGAFile(wPath.c_str(), DirectX::TGA_FLAGS_NONE, nullptr, image);
            if (FAILED(hr))
            {
                hr = DirectX::LoadFromDDSFile(wPath.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
                if (FAILED(hr))
                {
                    GLOG("Failed to load texture: %s", filePath);
                    return 0;
                }
            }
        }

        DirectX::Blob blob;
        hr = DirectX::SaveToDDSMemory(
            image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::DDS_FLAGS_NONE, blob
        );

        if (FAILED(hr))
        {
            GLOG("Failed to save texture in memory: %s", filePath);
            return 0;
        }

        UID textureUID       = GenerateUID();
        std::string savePath = TEXTURES_PATH + std::string("Texture") + TEXTURE_EXTENSION;
        UID finalTextureUID = App->GetLibraryModule()->AssignFiletypeUID(textureUID, savePath);
        std::string fileName = FileSystem::GetFileNameWithoutExtension(filePath);
        
        savePath = TEXTURES_PATH + fileName + TEXTURE_EXTENSION;

        MetaTexture meta(finalTextureUID, savePath, image.GetMetadata().mipLevels);
        meta.Save(savePath);


        unsigned int size =
            FileSystem::Save(savePath.c_str(), blob.GetBufferPointer(), (unsigned int)blob.GetBufferSize());

        if (size == 0)
        {
            GLOG("Failed to save DDS file: %s", savePath.c_str());
            return 0;
        }

        
        // added texture to textures map
        App->GetLibraryModule()->AddTexture(finalTextureUID, fileName);
        App->GetLibraryModule()->AddResource(savePath, finalTextureUID);

        GLOG("%s saved as dds", fileName.c_str());

        return finalTextureUID;
    }

    ResourceTexture* LoadTexture(UID textureUID)
    {
        std::string path   = App->GetLibraryModule()->GetResourcePath(textureUID);

        std::string fileName = FileSystem::GetFileNameWithoutExtension(path);
        //const UID loadedTextureUID = std::stoull(fileName);

        std::wstring wPath = std::wstring(path.begin(), path.end());

        DirectX::ScratchImage outImage;
        DirectX::TexMetadata outMetadata;

        HRESULT hr = LoadFromDDSFile(wPath.c_str(), DirectX::DDS_FLAGS_NONE, &outMetadata, outImage);

        if (FAILED(hr))
        {
            hr = LoadFromTGAFile(wPath.c_str(), DirectX::TGA_FLAGS_NONE, &outMetadata, outImage);
        }

        if (FAILED(hr))
        {
            hr = DirectX::LoadFromWICFile(wPath.c_str(), DirectX::WIC_FLAGS_NONE, &outMetadata, outImage);
        }

        if (FAILED(hr))
        {
            GLOG("Failed to load texture: %s", path.c_str());
            return nullptr;
        }

        ResourceTexture* texture = new ResourceTexture(textureUID, FileSystem::GetFileNameWithoutExtension(path));

        texture->LoadData(outMetadata, outImage);
        unsigned int textureID;
        glGenTextures(1, &textureID);

        glBindTexture(GL_TEXTURE_2D, textureID);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(outMetadata.mipLevels - 1));

        unsigned int internalFormat;
        unsigned int format;
        unsigned int type;
        switch (outMetadata.format)
        {
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_R8G8B8A8_UNORM:
            internalFormat = GL_RGBA8;
            format         = GL_RGBA;
            type           = GL_UNSIGNED_BYTE;
            //formatName     = "DXGI_FORMAT_R8G8B8A8_UNORM";
            break;
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            internalFormat = GL_RGBA8;
            format         = GL_BGRA;
            type           = GL_UNSIGNED_BYTE;
            //formatName     = "DXGI_FORMAT_B8G8R8A8_UNORM";
            break;
        case DXGI_FORMAT_B5G6R5_UNORM:
            internalFormat = GL_RGB8;
            format         = GL_BGR;
            type           = GL_UNSIGNED_BYTE;
            //formatName     = "DXGI_FORMAT_B5G6R5_UNORM";
            break;
        default:
            assert(false && "Unsupported format");
        }

        int maxMipmapLevel = static_cast<int>(outMetadata.mipLevels - 1);
        if (outMetadata.mipLevels > 1)
        {
            for (size_t i = 0; i < outMetadata.mipLevels; ++i)
            {
                const DirectX::Image* mip = outImage.GetImage(i, 0, 0);
                glTexImage2D(GL_TEXTURE_2D, static_cast<GLint>(i), internalFormat, static_cast<GLsizei>(mip->width), static_cast<GLsizei>(mip->height), 0, format, type, mip->pixels);
		    }
        }
        else
        {
            const DirectX::Image* baseImage = outImage.GetImage(0, 0, 0);
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, static_cast<GLsizei>(outMetadata.width), static_cast<GLsizei>(outMetadata.height), 0, format, type, baseImage->pixels);
            glGenerateMipmap(GL_TEXTURE_2D);
        }

        texture->SetTextureID(textureID);
        return texture;
    }

    unsigned int LoadCubemap(const char* texturePath)
    {
        unsigned int textureId = 0;
        std::string textureString(texturePath);
        std::wstring wPath = std::wstring(textureString.begin(), textureString.end());

        DirectX::ScratchImage scratchImage;
        DirectX::TexMetadata texMetadata;
        OpenGLMetadata openGlMeta;

        bool succeded = LoadTextureFile(wPath.c_str(), texMetadata, scratchImage);
        if (succeded)
        {
            ResourceTexture::ConvertMetadata(texMetadata, openGlMeta);

            glGenTextures(1, &textureId);
            glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

            // Sending texture to OpgenGL
            for (int i = 0; i < texMetadata.arraySize; ++i)
            {
                const DirectX::Image* face = scratchImage.GetImage(0, i, 0);
                glTexImage2D(
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, openGlMeta.internalFormat, static_cast<GLsizei>(face->width),
                    static_cast<GLsizei>(face->height), 0, openGlMeta.format, openGlMeta.type, face->pixels
                );
            }
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        }
        return textureId;
    }

    bool LoadTextureFile(const wchar_t* texturePath, DirectX::TexMetadata& outMetadata, DirectX::ScratchImage& outImage)
    {
        HRESULT hr = LoadFromDDSFile(texturePath, DirectX::DDS_FLAGS_NONE, &outMetadata, outImage);

        if (SUCCEEDED(hr)) return true;

        hr = LoadFromTGAFile(texturePath, DirectX::TGA_FLAGS_NONE, &outMetadata, outImage);

        if (SUCCEEDED(hr)) return true;

        hr = DirectX::LoadFromWICFile(texturePath, DirectX::WIC_FLAGS_NONE, &outMetadata, outImage);

        if (SUCCEEDED(hr)) return true;

        return false;
    }

}; // namespace TextureImporter
