#pragma once

#include "Globals.h"

#include <string>

namespace tinygltf
{
    class Model;
    struct Mesh;
    struct Primitive;
} // namespace tinygltf

enum DataType
{
    UNSIGNED_CHAR,
    UNSIGNED_SHORT,
    UNSIGNED_INT
};

class ResourceMesh;

namespace MeshImporter
{
    UID ImportMesh(
        const tinygltf::Model& model, const tinygltf::Mesh& mesh, const tinygltf::Primitive& primitive,
        const std::string& name, const char* filePath, const std::string& targetFilePath, UID sourceUID = INVALID_UID
    );
    ResourceMesh* LoadMesh(UID meshUID);
}; // namespace MeshImporter
