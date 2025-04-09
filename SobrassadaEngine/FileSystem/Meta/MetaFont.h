#pragma once

#include "MetaFile.h"

class MetaFont : public MetaFile
{
  public:
    MetaFont(UID uid, const std::string& assetPath);
    void AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const override;
};