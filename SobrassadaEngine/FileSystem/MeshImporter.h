#pragma once

#include "FileSystem.h"
#include "Globals.h"
#include "Mesh.h"
#include "ResourceManagement/Resources/ResourceMesh.h"

#include <memory>
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include "tiny_gltf.h"

enum DataType
{
    UNSIGNED_CHAR,
    UNSIGNED_SHORT,
    UNSIGNED_INT
};

namespace MeshImporter
{
    UID ImportMesh(
        const tinygltf::Model& model, const tinygltf::Mesh& mesh, const tinygltf::Primitive& primitive,
        const std::string& name, const char* filePath, const std::string& targetFilePath, UID sourceUID = INVALID_UID
    );
    ResourceMesh* LoadMesh(UID meshUID);
}; // namespace MeshImporter
