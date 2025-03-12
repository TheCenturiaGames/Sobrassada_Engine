#pragma once

#include "Globals.h"
#include "Math/float4x4.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include "tiny_gltf.h"

namespace SceneImporter
{
    void Import(const char* filePath);
    void ImportGLTF(const char* filePath);
    tinygltf::Model LoadModelGLTF(const char* filePath);
    void ImportMeshFromMetadata(const std::string& filePath, const std::string& name, UID sourceUID);
    void ImportMaterialFromMetadata(const std::string& filePath, const std::string& name, UID sourceUID);
    void CreateLibraryDirectories();

    float4x4 GetNodeTransform(const tinygltf::Node& node);
}; // namespace SceneImporter