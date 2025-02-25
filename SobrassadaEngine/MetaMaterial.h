#pragma once
#include "MetaFile.h"

class MetaMaterial : public MetaFile
{
  private:
    std::string shader;
    bool useAlpha;

  public:
    MetaMaterial(UID uid, const std::string& assetPath, const std::string& shader, bool useAlpha);
    void AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const override;
};