#pragma once
#include "MetaFile.h"

class MetaPrefab : public MetaFile
{
  public:
    MetaPrefab(UID uid, const std::string& assetPath);
    void AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const override;
};