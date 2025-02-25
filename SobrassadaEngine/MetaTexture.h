#pragma once

#include "MetaFile.h"

class MetaTexture : public MetaFile
{
  private:
    bool mipmaps;
    std::string compression;

  public:
    MetaTexture(UID uid, const std::string& assetPath, bool mipmaps, const std::string& compression);
    void AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const override;
};