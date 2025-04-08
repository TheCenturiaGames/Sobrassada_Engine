#pragma once

#include <Globals.h>

#include <string>

class ResourceFont;

namespace FontImporter
{
    UID ImportFont(const char* filePath, const std::string& targetFilePath);
    void CopyFont(
        const std::string& filePath, const std::string& targetFilePath, const std::string& name, const UID sourceUID
    );
    ResourceFont* LoadFont(UID fontUID);
} // namespace FontImporter
