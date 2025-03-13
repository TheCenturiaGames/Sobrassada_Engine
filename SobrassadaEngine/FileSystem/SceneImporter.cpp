#include "SceneImporter.h"

#include "Application.h"
#include "FileSystem.h"
#include "MaterialImporter.h"
#include "MeshImporter.h"
#include "ProjectModule.h"
#include "TextureImporter.h"
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
            std::string copyPath = App->GetProjectModule()->GetLoadedProjectPath() + ASSETS_PATH + FileSystem::GetFileNameWithExtension(filePath);
            if (!FileSystem::Exists(copyPath.c_str())) FileSystem::Copy(filePath, copyPath.c_str());
        }
        {
            std::string path = FileSystem::GetFilePath(filePath);

            // Copy bin to Assets folder
            for (const auto& srcBuffers : model.buffers)
            {
                std::string binPath     = path + srcBuffers.uri;
                std::string copyBinPath = App->GetProjectModule()->GetLoadedProjectPath() + ASSETS_PATH + FileSystem::GetFileNameWithExtension(binPath);
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
        const std::string convertedAssetPath = App->GetProjectModule()->GetLoadedProjectPath() + ASSETS_PATH;
        if (!FileSystem::IsDirectory(convertedAssetPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedAssetPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedAssetPath.c_str());
            }
        }
        const std::string convertedScenePath = App->GetProjectModule()->GetLoadedProjectPath() + SCENES_PATH;
        if (!FileSystem::IsDirectory(convertedScenePath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedScenePath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedScenePath.c_str());
            }
        }
        const std::string convertedMetadataPath = App->GetProjectModule()->GetLoadedProjectPath() + METADATA_PATH;
        if (!FileSystem::IsDirectory(convertedMetadataPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedMetadataPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedMetadataPath.c_str());
            }
        }
        const std::string convertedAnimationsPath = App->GetProjectModule()->GetLoadedProjectPath() + ANIMATIONS_PATH;
        if (!FileSystem::IsDirectory(convertedAnimationsPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedAnimationsPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedAnimationsPath.c_str());
            }
        }
        const std::string convertedAudioPath = App->GetProjectModule()->GetLoadedProjectPath() + AUDIO_PATH;
        if (!FileSystem::IsDirectory(convertedAudioPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedAudioPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedAudioPath.c_str());
            }
        }
        const std::string convertedBonesPath = App->GetProjectModule()->GetLoadedProjectPath() + MODELS_PATH;
        if (!FileSystem::IsDirectory(convertedBonesPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedBonesPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedBonesPath.c_str());
            }
        }
        const std::string convertedMeshesPath = App->GetProjectModule()->GetLoadedProjectPath() + MESHES_PATH;
        if (!FileSystem::IsDirectory(convertedMeshesPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedMeshesPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedMeshesPath.c_str());
            }
        }
        const std::string convertedPlayScenePath = App->GetProjectModule()->GetLoadedProjectPath() + SCENES_PLAY_PATH;
        if (!FileSystem::IsDirectory(convertedPlayScenePath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedPlayScenePath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedPlayScenePath.c_str());
            }
        }
        const std::string convertedTexturesPath = App->GetProjectModule()->GetLoadedProjectPath() + TEXTURES_PATH;
        if (!FileSystem::IsDirectory(convertedTexturesPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedTexturesPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedTexturesPath.c_str());
            }
        }
        const std::string convertedMaterialsPath = App->GetProjectModule()->GetLoadedProjectPath() + MATERIALS_PATH;
        if (!FileSystem::IsDirectory(convertedMaterialsPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedMaterialsPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedMaterialsPath.c_str());
            }
        }

    }
}; // namespace SceneImporter