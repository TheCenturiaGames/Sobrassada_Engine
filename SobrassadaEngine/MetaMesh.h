#pragma once
#include "MetaFile.h"


class MetaMesh : public MetaFile
{
  private:
    bool generateTangents;
    bool optimizeMesh;

  public:
    MetaMesh(UID uid, const std::string& assetPath, bool generateTangents, bool optimizeMesh);
    void AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const override;
};


