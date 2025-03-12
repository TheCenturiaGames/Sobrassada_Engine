#pragma once

#include "MetaFile.h"

class MetaTexture : public MetaFile
{
  public:
    MetaTexture(UID uid, const std::string& assetPath, int mipmaps);
    void AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const override;

  private:
    int mipmaps;
};