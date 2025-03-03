#pragma once
#include "MetaFile.h"

class MetaMaterial : public MetaFile
{
  private:
    std::string shader;
    bool useOcclusion;

  public:
    MetaMaterial(UID uid, const std::string& assetPath, const std::string& shader, bool useOcclusion);
    void AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const override;
};