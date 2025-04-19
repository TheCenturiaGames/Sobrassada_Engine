#pragma once

#include "MetaFile.h"

class NavMeshConfig;

class MetaNavmesh : public MetaFile
{
  public:
    MetaNavmesh(UID uid, const std::string& assetPath, const NavMeshConfig& config);
    void AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const override;
private:
    const NavMeshConfig& config;
};