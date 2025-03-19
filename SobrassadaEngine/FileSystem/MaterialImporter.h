#pragma once

#include "Globals.h"

#include <string>

namespace tinygltf
{
    class Model;
} // namespace tinygltf

class ResourceMaterial;

namespace MaterialImporter
{
    UID ImportMaterial(
        const tinygltf::Model& model, int materialIndex, const char* filePath, const std::string& targetFilePath,
        UID sourceUID = INVALID_UID
    );
    UID HandleTextureImport(const std::string& filePath, const std::string& targetFilePath);
    ResourceMaterial* LoadMaterial(UID materialUID);
} // namespace MaterialImporter
