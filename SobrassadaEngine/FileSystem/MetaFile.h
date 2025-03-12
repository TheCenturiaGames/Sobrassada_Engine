#pragma once

#include "Globals.h"
#include "Libs/rapidjson/document.h"
#include <ctime>
#include <string>

class MetaFile
{

  public:
    MetaFile(UID uid, const std::string& assetPath);
    virtual ~MetaFile() = default;

    virtual void AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const = 0;
    void Save(const std::string& name, const std::string& assetPath) const;

  private:
    UID assetUID;
    time_t lastModified;
};