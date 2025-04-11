#include "SceneImporter.h"

#include "Application.h"
#include "FileSystem.h"
#include "FontImporter.h"
#include "MaterialImporter.h"
#include "MeshImporter.h"
#include "ModelImporter.h"
#include "PrefabManager.h"
#include "ProjectModule.h"
#include "TextureImporter.h"

#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#define TINYGLTF_IMPLEMENTATION /* Only in one of the includes */
#include "tiny_gltf.h"
#include <utility>
#include <unordered_set>

namespace SceneImporter
{
    void Import(const char* filePath)
    {
        // TODO Is it necessary to create directories here? They should be already created
        const std::string engineDefaultPath = ENGINE_DEFAULT_ASSETS;
        CreateLibraryDirectories(App->GetProjectModule()->GetLoadedProjectPath());
        CreateLibraryDirectories(engineDefaultPath);

        std::string extension = FileSystem::GetFileExtension(filePath);

        if (extension == ASSET_EXTENSION) ImportGLTF(filePath, App->GetProjectModule()->GetLoadedProjectPath());
        else if (extension == FONT_EXTENSION)
            FontImporter::ImportFont(filePath, App->GetProjectModule()->GetLoadedProjectPath());
        else TextureImporter::Import(filePath, App->GetProjectModule()->GetLoadedProjectPath());
    }

    void ImportGLTF(const char* filePath, const std::string& targetFilePath)
    {
        tinygltf::Model model = LoadModelGLTF(filePath, targetFilePath);

        std::vector<std::vector<std::pair<UID, UID>>> gltfMeshes;
        std::unordered_map<int, UID> matIndices;

        for (const auto& srcMesh : model.meshes)
        {
            int n = 0;
            int matIndex = -1;
            std::vector<std::pair<UID, UID>> primitives;

            for (const auto& primitive : srcMesh.primitives)
            {
                std::string name = srcMesh.name + std::to_string(n);

                UID matUID   = INVALID_UID;
                matIndex = primitive.material;
                if (matIndex == -1)
                {
                    GLOG("Material index invalid for mesh: %s", name.c_str());
                }
                else if (matIndices.find(matIndex) == matIndices.end())
                {
                    matUID = MaterialImporter::ImportMaterial(model, matIndex, filePath, targetFilePath);
                    matIndices[matIndex] = matUID;
                }
                else
                {
                    matUID = matIndices[matIndex];
                }
                
                const UID meshUID      = MeshImporter::ImportMesh(model, srcMesh, primitive, name, filePath, targetFilePath, INVALID_UID, matUID);
                n++;
                
                primitives.emplace_back(meshUID, matUID);
                GLOG("New primitive with mesh UID: %d and Material UID: %d", meshUID, matUID);
            }
            gltfMeshes.push_back(primitives);
        }

        GLOG("Total .gltf meshes: %d", gltfMeshes.size());

        // Import Model
        ModelImporter::ImportModel(model, gltfMeshes, filePath, targetFilePath);
    }

    tinygltf::Model LoadModelGLTF(const char* filePath, const std::string& targetFilePath)
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
            std::string copyPath = targetFilePath + ASSETS_PATH + FileSystem::GetFileNameWithExtension(filePath);
            if (!FileSystem::Exists(copyPath.c_str())) FileSystem::Copy(filePath, copyPath.c_str());
        }
        {
            std::string path = FileSystem::GetFilePath(filePath);

            // Copy bin to Assets folder
            for (const auto& srcBuffers : model.buffers)
            {
                std::string binPath     = path + srcBuffers.uri;
                std::string copyBinPath = targetFilePath + ASSETS_PATH + FileSystem::GetFileNameWithExtension(binPath);
                if (!FileSystem::Exists(copyBinPath.c_str())) FileSystem::Copy(binPath.c_str(), copyBinPath.c_str());
            }
        }

        return model;
    }

    void ImportMeshFromMetadata(
        const std::string& filePath, const std::string& targetFilePath, const std::string& name, UID sourceUID
    )
    {
        tinygltf::Model model = LoadModelGLTF(filePath.c_str(), targetFilePath);

        std::string nameNoExt = name;
        if (!name.empty()) nameNoExt.pop_back(); // remove last character (extension)

        // find mesh name that equals to name
        for (const auto& srcMesh : model.meshes)
        {
            if (srcMesh.name == nameNoExt)
            {
                for (const auto& primitive : srcMesh.primitives)
                {
                    MeshImporter::ImportMesh(
                        model, srcMesh, primitive, name, filePath.c_str(), targetFilePath, sourceUID
                    );
                    return; // only one mesh with the same name
                }
            }
        }
    }

    void ImportMaterialFromMetadata(
        const std::string& filePath, const std::string& targetFilePath, const std::string& name, UID sourceUID
    )
    {
        tinygltf::Model model = LoadModelGLTF(filePath.c_str(), targetFilePath);

        // find material name that equals to name
        for (int i = 0; i < model.materials.size(); i++)
        {
            if (model.materials[i].name == name)
            {
                MaterialImporter::ImportMaterial(model, i, filePath.c_str(), targetFilePath, sourceUID);
                return; // only one material with the same name
            }
        }
    }

    void
    CopyPrefab(const std::string& filePath, const std::string& targetFilePath, const std::string& name, UID sourceUID)
    {
        PrefabManager::CopyPrefab(filePath, targetFilePath, name, sourceUID);
    }

    void
    CopyModel(const std::string& filePath, const std::string& targetFilePath, const std::string& name, UID sourceUID)
    {
        ModelImporter::CopyModel(filePath, targetFilePath, name, sourceUID);
    }

    void
    CopyFont(const std::string& filePath, const std::string& targetFilePath, const std::string& name, UID sourceUID)
    {
        FontImporter::CopyFont(filePath, targetFilePath, name, sourceUID);
    }

    void CreateLibraryDirectories(const std::string& projectFilePath)
    {
        const std::string convertedAssetPath = projectFilePath + ASSETS_PATH;
        if (!FileSystem::IsDirectory(convertedAssetPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedAssetPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedAssetPath.c_str());
            }
        }
        const std::string convertedScenePath = projectFilePath + SCENES_PATH;
        if (!FileSystem::IsDirectory(convertedScenePath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedScenePath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedScenePath.c_str());
            }
        }
        const std::string convertedModelAssetsPath = projectFilePath + MODELS_ASSETS_PATH;
        if (!FileSystem::IsDirectory(convertedModelAssetsPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedModelAssetsPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedModelAssetsPath.c_str());
            }
        }
        const std::string convertedMetadataPath = projectFilePath + METADATA_PATH;
        if (!FileSystem::IsDirectory(convertedMetadataPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedMetadataPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedMetadataPath.c_str());
            }
        }
        const std::string convertedPrefabAssetsPath = projectFilePath + PREFABS_ASSETS_PATH;
        if (!FileSystem::IsDirectory(convertedPrefabAssetsPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedPrefabAssetsPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedPrefabAssetsPath.c_str());
            }
        }
        const std::string convertedStateMachinePath = projectFilePath + STATEMACHINES_ASSETS_PATH;
        if (!FileSystem::IsDirectory(convertedStateMachinePath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedStateMachinePath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedStateMachinePath.c_str());
            }
        }
        const std::string convertedAnimationsPath = projectFilePath + ANIMATIONS_PATH;
        if (!FileSystem::IsDirectory(convertedAnimationsPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedAnimationsPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedAnimationsPath.c_str());
            }
        }
        const std::string convertedAudioPath = projectFilePath + AUDIO_PATH;
        if (!FileSystem::IsDirectory(convertedAudioPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedAudioPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedAudioPath.c_str());
            }
        }
        const std::string convertedBonesPath = projectFilePath + MODELS_LIB_PATH;
        if (!FileSystem::IsDirectory(convertedBonesPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedBonesPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedBonesPath.c_str());
            }
        }
        const std::string convertedMeshesPath = projectFilePath + MESHES_PATH;
        if (!FileSystem::IsDirectory(convertedMeshesPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedMeshesPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedMeshesPath.c_str());
            }
        }
        const std::string convertedPlayScenePath = projectFilePath + SCENES_PLAY_PATH;
        if (!FileSystem::IsDirectory(convertedPlayScenePath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedPlayScenePath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedPlayScenePath.c_str());
            }
        }
        const std::string convertedTexturesPath = projectFilePath + TEXTURES_PATH;
        if (!FileSystem::IsDirectory(convertedTexturesPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedTexturesPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedTexturesPath.c_str());
            }
        }
        const std::string convertedMaterialsPath = projectFilePath + MATERIALS_PATH;
        if (!FileSystem::IsDirectory(convertedMaterialsPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedMaterialsPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedMaterialsPath.c_str());
            }
        }
        const std::string convertedPrefabLibraryPath = projectFilePath + PREFABS_LIB_PATH;
        if (!FileSystem::IsDirectory(convertedPrefabLibraryPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedPrefabLibraryPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedPrefabLibraryPath.c_str());
            }
        }
        const std::string convertedStateMachineLibraryPath = projectFilePath + STATEMACHINES_LIB_PATH;
        if (!FileSystem::IsDirectory(convertedStateMachineLibraryPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedStateMachineLibraryPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedStateMachineLibraryPath.c_str());
            }
        }
        const std::string convertedFontsPath = projectFilePath + FONTS_PATH;
        if (!FileSystem::IsDirectory(convertedFontsPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedFontsPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedFontsPath.c_str());
            }
        }
    }
}; // namespace SceneImporter