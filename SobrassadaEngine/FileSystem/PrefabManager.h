#pragma once

#include "Globals.h"

class ResourcePrefab;
class GameObject;

namespace PrefabManager
{
    UID SavePrefab(const GameObject* gameObject);
    ResourcePrefab* LoadPrefab(UID prefabUID);
}
