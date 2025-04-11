#pragma once

#include "MetaFile.h"

class MetaAnimation : public MetaFile
{
  public:
	MetaAnimation(UID uid, const std::string& assetPath);
    void AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const override;

	
};
