#pragma once

#include "Globals.h"
#include "ResourceManagement/Resources/ResourceMaterial.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include "tiny_gltf.h"
#include <string>

namespace MaterialImporter
{
    UID
    ImportMaterial(const tinygltf::Model& model, int materialIndex, const char* filePath, UID sourceUID = INVALID_UID);
    UID HandleTextureImport(const std::string& filePath);
    ResourceMaterial* LoadMaterial(UID materialUID);
} // namespace MaterialImporter
