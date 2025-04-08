#pragma once

#include "MetaFile.h"

#include "Math/float4x4.h"

class MetaMesh : public MetaFile
{
  public:
    MetaMesh(UID uid, const std::string& assetPath, bool generateTangents, const float4x4& transform, UID defaultMaterialUID);
    void AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const override;

  private:
    bool generateTangents;
    float4x4 transform;

    UID defaultMaterialUID = INVALID_UID;
};
