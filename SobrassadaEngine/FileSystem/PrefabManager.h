#pragma once

#include "Globals.h"
#include "ResourceManagement/Resources/ResourcePrefab.h"

class ResourcePrefab;
class GameObject;

namespace PrefabManager
{
    UID SavePrefab(const GameObject* gameObject, bool override);
    void CopyPrefab(
        const std::string& filePath, const std::string& targetFilePath, const std::string& name, const UID sourceUID
    );
    ResourcePrefab* LoadPrefab(UID prefabUID);
}
