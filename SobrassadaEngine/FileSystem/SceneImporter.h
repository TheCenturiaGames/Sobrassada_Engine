#pragma once

#include "Globals.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include "tiny_gltf.h"

namespace SceneImporter
{
    void Import(const char* filePath);
    void ImportGLTF(const char* filePath, const std::string& targetFilePath);
    tinygltf::Model LoadModelGLTF(const char* filePath, const std::string& targetFilePath);
    void ImportMeshFromMetadata(const std::string& filePath, const std::string& targetFilePath, const std::string& name, UID sourceUID);
    void ImportMaterialFromMetadata(const std::string& filePath, const std::string& targetFilePath, const std::string& name, UID sourceUID);
    void CopyPrefab(const std::string& filePath, const std::string& name, UID sourceUID);
    void ImportModelFromMetadata(const std::string& filePath, const std::string& targetFilePath, const std::string& name, UID sourceUID);
    void CreateLibraryDirectories(const std::string& projectFilePath);
}; // namespace SceneImporter