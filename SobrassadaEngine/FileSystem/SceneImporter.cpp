#include "SceneImporter.h"

#include "FileSystem.h"
#include "MaterialImporter.h"
#include "MeshImporter.h"
#include "TextureImporter.h"

#include "Math/Quat.h"
#include "ModelImporter.h"

namespace SceneImporter
{
    void Import(const char* filePath)
    {
        CreateLibraryDirectories();

        std::string extension = FileSystem::GetFileExtension(filePath);

        if (extension == ASSET_EXTENSION) ImportGLTF(filePath);
        else TextureImporter::Import(filePath);
    }

    void ImportGLTF(const char* filePath)
    {
        tinygltf::Model model = LoadModelGLTF(filePath);

        std::vector<std::vector<std::pair<UID, UID>>> gltfMeshes;
        std::vector<int> matIndices;
        for (const auto& srcMesh : model.meshes)
        {
            int n            = 0;
            int matIndex     = -1;
            std::vector<std::pair<UID, UID>> primitives;

            for (const auto& primitive : srcMesh.primitives)
            {
                std::string name = srcMesh.name + std::to_string(n);
                UID meshUID = MeshImporter::ImportMesh(model, srcMesh, primitive, name, filePath);
                n++;

                UID matUID = INVALID_UID;
                matIndex = primitive.material;
                if (matIndex == -1) GLOG("Material index invalid for mesh: %s", name.c_str())
                else if (std::find(matIndices.begin(), matIndices.end(), matIndex) == matIndices.end())
                {
                    matUID = MaterialImporter::ImportMaterial(model, matIndex, filePath);
                    matIndices.push_back(matIndex);
                }

                primitives.emplace_back(meshUID, matUID);
                GLOG("New primitive with mesh UID: %d and Material UID: %d", meshUID, matUID);
            }
            gltfMeshes.emplace_back(primitives);
        }

         GLOG("Total .gltf meshes: %d", gltfMeshes.size());

        // Import Model
        ModelImporter::ImportModel(model.nodes, gltfMeshes, filePath);
    }

    tinygltf::Model LoadModelGLTF(const char* filePath)
    {
        tinygltf::TinyGLTF gltfContext;
        tinygltf::Model model;
        std::string err, warn;

        bool ret = gltfContext.LoadASCIIFromFile(&model, &err, &warn, std::string(filePath));
        if (!ret)
        {
            if (!warn.empty())
            {
                GLOG("Warn: %s\n", warn.c_str());
            }

            if (!err.empty())
            {
                GLOG("Err: %s\n", err.c_str());
            }

            if (!ret)
            {
                GLOG("Failed to parse glTF\n");
            }
        }

        {
            // Copy gltf to Assets folder
            std::string copyPath = ASSETS_PATH + FileSystem::GetFileNameWithExtension(filePath);
            if (!FileSystem::Exists(copyPath.c_str())) FileSystem::Copy(filePath, copyPath.c_str());
        }
        {
            std::string path = FileSystem::GetFilePath(filePath);

            std::vector<float4x4> nodeTransforms(model.nodes.size(), float4x4::identity);

            for (size_t i = 0; i < model.nodes.size(); ++i)
            {
                const tinygltf::Node& node = model.nodes[i];

                float4x4 localTransform    = float4x4::identity;

                localTransform             = GetNodeTransform(node);

                nodeTransforms[i]          = nodeTransforms[i] * localTransform;
                // multiply parent by all children
                if (node.children.size() > 0)
                {
                    for (size_t j = 0; j < node.children.size(); ++j)
                    {
                        nodeTransforms[node.children[j]] = nodeTransforms[i] * nodeTransforms[node.children[j]];
                    }
                }

                if (node.mesh >= 0)
                {

                    int meshnode               = node.mesh;
                    const tinygltf::Mesh& mesh = model.meshes[node.mesh];
                    int n                      = 0;
                    int matIndex               = 0;

                    for (const auto& primitive : mesh.primitives)
                    {
                        matIndex             = primitive.material;
                        std::string meshName = mesh.name + std::to_string(n);
                        MeshImporter::ImportMesh(model, mesh, primitive, meshName, filePath, nodeTransforms[i]);
                        MaterialImporter::ImportMaterial(model, matIndex, filePath);
                        n++;
                    }
                }
            }
            // Copy bin to Assets folder
            for (const auto& srcBuffers : model.buffers)
            {
                std::string binPath     = path + srcBuffers.uri;
                std::string copyBinPath = ASSETS_PATH + FileSystem::GetFileNameWithExtension(binPath);
                if (!FileSystem::Exists(copyBinPath.c_str())) FileSystem::Copy(binPath.c_str(), copyBinPath.c_str());
            }
        }

        return model;
    }

    void ImportMeshFromMetadata(const std::string& filePath, const std::string& name, UID sourceUID)
    {
        tinygltf::Model model = LoadModelGLTF(filePath.c_str());

        std::string nameNoExt = name;
        if (!name.empty()) nameNoExt.pop_back(); // remove last character (extension)

        // find mesh name that equals to name
        for (const auto& srcMesh : model.meshes)
        {
            if (srcMesh.name == nameNoExt)
            {
                for (const auto& primitive : srcMesh.primitives)
                {
                    MeshImporter::ImportMesh(model, srcMesh, primitive, name, filePath.c_str(), sourceUID);
                    return; // only one mesh with the same name
                }
            }
        }
    }

    void ImportMaterialFromMetadata(const std::string& filePath, const std::string& name, UID sourceUID)
    {
        tinygltf::Model model = LoadModelGLTF(filePath.c_str());

        // find material name that equals to name
        for (int i = 0; i < model.materials.size(); i++)
        {
            if (model.materials[i].name == name)
            {
                MaterialImporter::ImportMaterial(model, i, filePath.c_str(), sourceUID);
                return; // only one material with the same name
            }
        }
    }

    void CreateLibraryDirectories()
    {
        if (!FileSystem::IsDirectory(ASSETS_PATH))
        {
            if (!FileSystem::CreateDirectories(ASSETS_PATH))
            {
                GLOG("Failed to create directory: %s", ASSETS_PATH);
            }
        }
        if (!FileSystem::IsDirectory(SCENES_PATH))
        {
            if (!FileSystem::CreateDirectories(SCENES_PATH))
            {
                GLOG("Failed to create directory: %s", SCENES_PATH);
            }
        }
        if (!FileSystem::IsDirectory(METADATA_PATH))
        {
            if (!FileSystem::CreateDirectories(METADATA_PATH))
            {
                GLOG("Failed to create directory: %s", METADATA_PATH);
            }
        }
        if (!FileSystem::IsDirectory(ANIMATIONS_PATH))
        {
            if (!FileSystem::CreateDirectories(ANIMATIONS_PATH))
            {
                GLOG("Failed to create directory: %s", ANIMATIONS_PATH);
            }
        }
        if (!FileSystem::IsDirectory(AUDIO_PATH))
        {
            if (!FileSystem::CreateDirectories(AUDIO_PATH))
            {
                GLOG("Failed to create directory: %s", AUDIO_PATH);
            }
        }
        if (!FileSystem::IsDirectory(MODELS_PATH))
        {
            if (!FileSystem::CreateDirectories(MODELS_PATH))
            {
                GLOG("Failed to create directory: %s", MODELS_PATH);
            }
        }
        if (!FileSystem::IsDirectory(MESHES_PATH))
        {
            if (!FileSystem::CreateDirectories(MESHES_PATH))
            {
                GLOG("Failed to create directory: %s", MESHES_PATH);
            }
        }
        if (!FileSystem::IsDirectory(SCENES_PLAY_PATH))
        {
            if (!FileSystem::CreateDirectories(SCENES_PLAY_PATH))
            {
                GLOG("Failed to create directory: %s", SCENES_PLAY_PATH);
            }
        }
        if (!FileSystem::IsDirectory(TEXTURES_PATH))
        {
            if (!FileSystem::CreateDirectories(TEXTURES_PATH))
            {
                GLOG("Failed to create directory: %s", TEXTURES_PATH);
            }
        }
        if (!FileSystem::IsDirectory(MATERIALS_PATH))
        {
            if (!FileSystem::CreateDirectories(MATERIALS_PATH))
            {
                GLOG("Failed to create directory: %s", MATERIALS_PATH);
            }
        }

    }

    float4x4 GetNodeTransform(const tinygltf::Node& node)
    {

        if (!node.matrix.empty())
        {
            // glTF stores matrices in COLUMN-MAJOR order, same as MathGeoLib
            return float4x4(
                (float)node.matrix[0], (float)node.matrix[1], (float)node.matrix[2], (float)node.matrix[3],
                (float)node.matrix[4], (float)node.matrix[5], (float)node.matrix[6], (float)node.matrix[7],
                (float)node.matrix[8], (float)node.matrix[9], (float)node.matrix[10], (float)node.matrix[11],
                (float)node.matrix[12], (float)node.matrix[13], (float)node.matrix[14], (float)node.matrix[15]
            );
        }

        // Default values
        float3 translation = float3::zero;
        Quat rotation      = Quat::identity;
        float3 scale       = float3::one;

        if (!node.translation.empty())
            translation = float3((float)node.translation[0], (float)node.translation[1], (float)node.translation[2]);

        if (!node.rotation.empty())
            rotation = Quat(
                (float)node.rotation[0], (float)node.rotation[1], (float)node.rotation[2], (float)node.rotation[3]
            ); // glTF stores as [x, y, z, w]

        if (!node.scale.empty()) scale = float3((float)node.scale[0], (float)node.scale[1], (float)node.scale[2]);

        return float4x4::FromTRS(translation, rotation, scale);
    }
}; // namespace SceneImporter