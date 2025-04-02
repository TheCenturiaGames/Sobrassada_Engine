#pragma once

#include "Resource.h"

#include <Math/float3.h>
#include <Math/float4.h>

class Material;

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
    float4 diffColor      = {1.0f, 0.0f, 0.0f, 1.0f};
    float3 specColor      = {1.0f, 0.0f, 0.0f};
    float shininess       = 500.0f;
    bool shininessInAlpha = false;
    float metallicFactor  = 1.0f;
    float roughnessFactor = 1.0f;
    UID diffuseTex        = 0;
    UID specularTex       = 0;
    UID metallicTex       = 0;
    UID normalTex         = 0;
};

class Material;

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
    UID ChangeTexture(UID newTexture, TextureInfo& textureToChange, UID textureGPU);

  private:
    TextureInfo diffuseTexture;
    TextureInfo specularTexture;
    TextureInfo metallicTexture;
    TextureInfo normalTexture;

    MaterialGPU material;

    unsigned int ubo = 0;
};
