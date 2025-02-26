#pragma once

#include "Resource.h"

#include "DirectXTex/DirectXTex.h"

struct OpenGLMetadata
{
    unsigned internalFormat;
    unsigned format;
    unsigned type;
};

namespace DirectX
{
    struct TexMetadata;
    class ScratchImage;
} // namespace DirectX

class ResourceTexture : public Resource
{
  public:
    ResourceTexture(UID uid, const std::string& name);
    ~ResourceTexture() override;

    void LoadData(const DirectX::TexMetadata& metadata, const DirectX::ScratchImage& scratchImage);
    static void ConvertMetadata(const DirectX::TexMetadata& metadata, OpenGLMetadata& outMetadata);

    unsigned int GetTextureID() { return textureID; }

    void SetTextureID(unsigned int id) { textureID = id; }

  private:
    unsigned int textureID = 0;
    DirectX::ScratchImage scratchImage;
    DirectX::TexMetadata metadata;
    OpenGLMetadata openGLMetadata;
};
