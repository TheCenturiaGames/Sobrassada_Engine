#pragma once

#include "MetaFile.h"

class MetaStateMachine : public MetaFile
{
  public:
    MetaStateMachine(UID uid, const std::string& assetPath);
    void AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const override;
};