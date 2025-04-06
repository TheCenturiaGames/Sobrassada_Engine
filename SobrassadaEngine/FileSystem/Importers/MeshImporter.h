#pragma once

#include "Globals.h"

#include "Math/float4x4.h"
#include <string>

class ResourceMesh;

namespace tinygltf
{
    class Model;
    struct Mesh;
    struct Primitive;
    class Node;
} // namespace tinygltf

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
        const std::string& name, const char* filePath, const std::string& targetFilePath, UID sourceUID = INVALID_UID, UID defaultMatUID = INVALID_UID
    );
    const float4x4 GetMeshDefaultTransform(const tinygltf::Model& model, const std::string& name);
    const float4x4 GetNodeTransform(const tinygltf::Node& node);
    ResourceMesh* LoadMesh(UID meshUID);
}; // namespace MeshImporter
