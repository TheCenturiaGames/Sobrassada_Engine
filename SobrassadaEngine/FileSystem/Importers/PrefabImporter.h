#pragma once

#include "Globals.h"

#include <string>

class ResourcePrefab;
class GameObject;

namespace PrefabImporter
{

    UID Import(const char* filePath, const std::string& targetFilePath, UID sourceUID = INVALID_UID);
    UID SavePrefab(const GameObject* gameObject, bool override);
    void CopyPrefab(
        const std::string& filePath, const std::string& targetFilePath, const std::string& name, const UID sourceUID
    );
    ResourcePrefab* LoadPrefab(UID prefabUID);
};