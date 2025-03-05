#pragma once

#include "Math/float3.h"
#include "Math/float4.h"
#include <string>
#include <vector>

class Material
{
  public:
    Material() = default;

    // Getters (const refs or pointers to avoid copies)
    const float4& GetDiffuseFactor() const { return diffuseFactor; }
    const float3& GetSpecularFactor() const { return specularFactor; }
    float GetGlossinessFactor() const { return glossinessFactor; }
    float GetOcclusionStrength() const { return occlusionStrength; }

    const UID GetDiffuseTexture() const { return diffuseTexture; }
    const UID GetSpecularGlossinessTexture() const { return specularGlossinessTexture; }
    const UID GetNormalTexture() const { return normalTexture; }
    const UID GetOcclusionTexture() const { return occlusionTexture; }

    const UID GetMaterialUID() const { return materialUID; }

    // Setters (use move & pointers to avoid copies)
    void SetDiffuseFactor(const float4& newDiffuseFactor) { diffuseFactor = newDiffuseFactor; }
    void SetSpecularFactor(const float3& newSpecularFactor) { specularFactor = newSpecularFactor; }
    void SetGlossinessFactor(float newGlossiness) { glossinessFactor = newGlossiness; }
    void SetOcclusionStrength(float strength) { occlusionStrength = strength; }
    void SetMetallicFactor(float newMetallicFactor) { metallicFactor = newMetallicFactor; }
    void SetRoughnessFactor(float newRoughnessFactor) { roughnessFactor = newRoughnessFactor; }


    void SetSpecularGlossinessTexture(UID texture) { specularGlossinessTexture = texture; }
    void SetNormalTexture(UID texture) { normalTexture = texture; }
    void SetDiffuseTexture(UID texture) { diffuseTexture = texture; }
    void SetOcclusionTexture(UID texture) { occlusionTexture = texture; }

    void SetMaterialUID(UID uid) { materialUID = uid; }

  private:
    float4 diffuseFactor                  = {1.0f, 1.0f, 1.0f, 1.0f}; // RGBA
    float3 specularFactor                 = {1.0f, 1.0f, 1.0f};       // RGB
    float glossinessFactor                = 1.0f;
    float occlusionStrength               = 1.0f;

    // pointers to the dds paths
    UID diffuseTexture            = CONSTANT_EMPTY_UID;
    //UID metallicRoughnessTexture  = CONSTANT_EMPTY_UID;
    UID specularGlossinessTexture = CONSTANT_EMPTY_UID;
    UID normalTexture             = CONSTANT_EMPTY_UID;
    UID occlusionTexture          = CONSTANT_EMPTY_UID;

    UID materialUID               = CONSTANT_EMPTY_UID;

    //need to be here for not breaking the .mat existing (when loading) 
    float metallicFactor                  = 0.0f;
    float roughnessFactor                 = 1.0f;
    UID metallicRoughnessTexture  = CONSTANT_EMPTY_UID;
};