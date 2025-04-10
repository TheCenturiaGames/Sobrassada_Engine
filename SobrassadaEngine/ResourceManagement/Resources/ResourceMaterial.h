﻿#pragma once

#include "Resource.h"

#include "Math/float3.h"
#include "Math/float4.h"
#include "rapidjson/document.h"

class Material;

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
    uint64_t diffuseTex   = 0;
    uint64_t specularTex  = 0;
    uint64_t metallicTex  = 0;
    uint64_t normalTex    = 0;
};

class ResourceMaterial : public Resource
{
  public:
    ResourceMaterial(UID uid, const std::string& name, const rapidjson::Value& importOptions);
    ~ResourceMaterial() override;

    void OnEditorUpdate();
    void LoadMaterialData(Material mat);
    void FreeMaterials() const;

    UID ChangeTexture(UID newTexture, TextureInfo& textureToChange, UID textureGPU);
    void ChangeFallBackTexture();

    const bool GetIsMetallicRoughness() const { return metallicTexture.textureID != 0 ? true : false; }
    const MaterialGPU GetMaterial() const { return material; }

  private:
    TextureInfo diffuseTexture;
    TextureInfo specularTexture;
    TextureInfo metallicTexture;
    TextureInfo normalTexture;

    MaterialGPU material;
    UID defaultTextureUID = INVALID_UID;
};
