#pragma once

#include "MetaFile.h"

#include "Math/float4x4.h"

class MetaMesh : public MetaFile
{
  public:
    MetaMesh(
        UID uid, const std::string& assetPath, bool generateTangents, const float4x4& transform, UID defaultMaterialUID,
        uint32_t gltfMeshIndex, uint32_t gltfPrimitiveIndex
    );
    void AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const override;

    uint32_t GetGltfMeshIndex() const { return gltfMeshIndex; }

    uint32_t GetGltfPrimitiveIndex() const { return gltfPrimitiveIndex; }

    const float4x4& GetTransform() const { return transform; }

  private:
    uint32_t gltfMeshIndex;
    uint32_t gltfPrimitiveIndex;
    bool generateTangents;
    float4x4 transform;

    UID defaultMaterialUID = INVALID_UID;
};
