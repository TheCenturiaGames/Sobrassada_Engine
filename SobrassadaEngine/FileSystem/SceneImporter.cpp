#include "SceneImporter.h"

#include "Application.h"
#include "FileSystem.h"
#include "MaterialImporter.h"
#include "MeshImporter.h"
#include "ProjectModule.h"
#include "TextureImporter.h"

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

        std::vector<int> matIndices;
        for (const auto& srcMesh : model.meshes)
        {
            std::string name = srcMesh.name;
            int n            = 0;
            int matIndex     = -1;
            for (const auto& primitive : srcMesh.primitives)
            {
                name += std::to_string(n);
                MeshImporter::ImportMesh(model, srcMesh, primitive, name, filePath);
                n++;

                matIndex = primitive.material;
                if (matIndex == -1) GLOG("Material index invalid for mesh: %s", name.c_str())
                else if (std::find(matIndices.begin(), matIndices.end(), matIndex) == matIndices.end())
                {
                    MaterialImporter::ImportMaterial(model, matIndex, filePath);
                    matIndices.push_back(matIndex);
                }
            }
        }
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
        const std::string projectWorkingDir = App->GetProjectModule()->GetLoadedProjectPath() + DELIMITER;
        const std::string convertedAssetPath = projectWorkingDir + ASSETS_PATH;
        if (!FileSystem::IsDirectory(convertedAssetPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedAssetPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedAssetPath.c_str());
            }
        }
        const std::string convertedMetadataPath = projectWorkingDir + METADATA_PATH;
        if (!FileSystem::IsDirectory(convertedMetadataPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedMetadataPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedMetadataPath.c_str());
            }
        }
        const std::string convertedAnimationsPath = projectWorkingDir + ANIMATIONS_PATH;
        if (!FileSystem::IsDirectory(convertedAnimationsPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedAnimationsPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedAnimationsPath.c_str());
            }
        }
        const std::string convertedAudioPath = projectWorkingDir + AUDIO_PATH;
        if (!FileSystem::IsDirectory(convertedAudioPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedAudioPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedAudioPath.c_str());
            }
        }
        const std::string convertedBonesPath = projectWorkingDir + BONES_PATH;
        if (!FileSystem::IsDirectory(convertedBonesPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedBonesPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedBonesPath.c_str());
            }
        }
        const std::string convertedMeshesPath = projectWorkingDir + MESHES_PATH;
        if (!FileSystem::IsDirectory(convertedMeshesPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedMeshesPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedMeshesPath.c_str());
            }
        }
        const std::string convertedTexturesPath = projectWorkingDir + TEXTURES_PATH;
        if (!FileSystem::IsDirectory(convertedTexturesPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedTexturesPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedTexturesPath.c_str());
            }
        }
        const std::string convertedMaterialsPath = projectWorkingDir + MATERIALS_PATH;
        if (!FileSystem::IsDirectory(convertedMaterialsPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedMaterialsPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedMaterialsPath.c_str());
            }
        }
        const std::string convertedScenesPath = projectWorkingDir + SCENES_PATH;
        if (!FileSystem::IsDirectory(convertedScenesPath.c_str()))
        {
            if (!FileSystem::CreateDirectories(convertedScenesPath.c_str()))
            {
                GLOG("Failed to create directory: %s", convertedScenesPath.c_str());
            }
        }
    }
}; // namespace SceneImporter