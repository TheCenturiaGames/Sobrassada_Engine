#include "PrefabImporter.h"

#include "FileSystem.h"
#include <MetaPrefab.h>

namespace PrefabImporter
{
    UID Import(const char* sourceFilePath, const std::string& targetFilePath, UID sourceUID)
    {
        const std::string fileName     = FileSystem::GetFileNameWithExtension(sourceFilePath);
        const std::string relativePath = PREFABS_ASSETS_PATH + fileName;
        const std::string copyPath     = targetFilePath + relativePath;

        if (!FileSystem::Exists(copyPath.c_str()))
        {
            FileSystem::Copy(sourceFilePath, copyPath.c_str());
        }
    }

    ResourcePrefab* LoadPrefab(UID textureUID)
    {
    }

} // namespace PrefabImporter