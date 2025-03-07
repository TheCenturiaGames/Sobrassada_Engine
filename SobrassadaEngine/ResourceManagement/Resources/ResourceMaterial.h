#pragma once

#include "Resource.h"
#include "FileSystem/Material.h"

#include <Math/float4.h>

namespace tinygltf
{
    class Model;
    struct Material;
} // namespace tinygltf

struct TextureInfo
{
    unsigned int textureID = 0;
    int width              = 0;
    int height             = 0;
};

struct MaterialGPU
{
    float4 diffColor     = {1.0f, 0.0f, 0.0f, 1.0f};
    float3 specColor     = {1.0f, 0.0f, 0.0f};
    float shininess      = 500.0f;
    bool shininessInAlpha = false;
    float metallicFactor  = 1.0f;
    float roughnessFactor = 1.0f;
    uint64_t diffuseTex = 0;
    uint64_t specularTex = 0;
    uint64_t metallicTex = 0;
    uint64_t normalTex = 0;
};

class ResourceMaterial : public Resource
{
  public:
    ResourceMaterial(UID uid, const std::string& name);
    ~ResourceMaterial() override;

    const bool GetIsMetallicRoughness() const { return metallicTexture.textureID != 0 ? true : false; }

    void OnEditorUpdate();
    void LoadMaterialData(Material mat);
    void RenderMaterial(int program) const;
    void FreeMaterials() const;
    void UpdateUBO() const;

  private:
    TextureInfo diffuseTexture;
    TextureInfo specularTexture;
    TextureInfo metallicTexture;
    TextureInfo normalTexture;

    MaterialGPU material;

    unsigned int ubo = 0;
};
