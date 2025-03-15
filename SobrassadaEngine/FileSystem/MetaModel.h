#pragma once

#include "MetaFile.h"

class MetaModel : public MetaFile
{
  public:
    MetaModel(UID uid, const std::string& assetPath);
    void AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const override;

};