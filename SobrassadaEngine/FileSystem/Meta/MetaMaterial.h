#pragma once

#include "MetaFile.h"

class MetaMaterial : public MetaFile
{
  public:
    MetaMaterial(UID uid, const std::string& assetPath, const std::string& shader, bool useOcclusion, UID defaultTextureUID);
    void AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const override;

  private:
    std::string shader;
    bool useOcclusion;

    UID defaultTextureUID = INVALID_UID;
};