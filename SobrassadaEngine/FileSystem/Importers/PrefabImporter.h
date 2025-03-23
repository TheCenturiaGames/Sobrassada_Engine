#pragma once

#include <Globals.h>


class ResourcePrefab;

namespace PrefabImporter{

	UID Import(const char* filePath, const std::string& targetFilePath, UID sourceUID = INVALID_UID);
    ResourcePrefab* LoadPrefab(UID textureUID);


};