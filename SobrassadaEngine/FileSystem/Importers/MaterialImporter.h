#pragma once

#include "Globals.h"

#include <string>

class ResourceMaterial;

namespace tinygltf
{
    class Model;
} // namespace tinygltf

namespace MaterialImporter
{
    UID ImportMaterial(
        const tinygltf::Model& model, int materialIndex, const char* filePath, const std::string& targetFilePath,
        UID sourceUID = INVALID_UID, UID defaultTextureUID = INVALID_UID
    );
    UID HandleTextureImport(const std::string& filePath, const std::string& targetFilePath);
    ResourceMaterial* LoadMaterial(UID materialUID);
} // namespace MaterialImporter
