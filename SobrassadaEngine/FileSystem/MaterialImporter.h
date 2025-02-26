#pragma once

#include "Globals.h"
#include "ResourceManagement/Resources/ResourceMaterial.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include "tiny_gltf.h"

namespace MaterialImporter
{
    UID ImportMaterial(const tinygltf::Model& model, int materialIndex, const char* filePath);
    ResourceMaterial* LoadMaterial(UID materialUID);
} // namespace MaterialImporter
