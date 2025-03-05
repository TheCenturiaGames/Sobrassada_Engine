#pragma once

#include "Globals.h"
#include "ResourceManagement/Resources/ResourcePrefab.h"

class ResourcePrefab;
class GameObject;

namespace PrefabManager
{
    UID SavePrefab(const GameObject* gameObject);
    ResourcePrefab* LoadPrefab(UID prefabUID);
}
