#include "LibraryModule.h"

#include "Application.h"
#include "Component.h"
#include "ComponentUtils.h"
#include "FileSystem.h"
#include "GameObject.h"
#include "SceneImporter.h"
#include "SceneModule.h"
#include "TextureImporter.h"

#include "Libs/rapidjson/stringbuffer.h"
#include "Libs/rapidjson/writer.h"
#include "document.h"
#include "prettywriter.h"
#include "stringbuffer.h"
#include <filesystem>
#include <fstream>

LibraryModule::LibraryModule()
{
}

LibraryModule::~LibraryModule()
{
}

bool LibraryModule::Init()
{
    SceneImporter::CreateLibraryDirectories();
    LibraryModule::LoadLibraryMaps();

    return true;
}

// Save = library/scenes/
// SaveAs = all path + name.scene
bool LibraryModule::SaveScene(const char* path, SaveMode saveMode) const
{
    Scene* loadedScene = App->GetSceneModule()->GetScene();
    if (loadedScene != nullptr)
    {
        const char* sceneName = loadedScene->GetSceneName();
        if (sceneName == DEFAULT_SCENE_NAME && saveMode == SaveMode::Save) return false;
        // Create doc JSON
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        rapidjson::Value scene(rapidjson::kObjectType);

        loadedScene->Save(scene, allocator);

        doc.AddMember("Scene", scene, allocator);

        // Save file like JSON
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        std::string sceneFilePath;
        std::string fileName;

        if (saveMode == SaveMode::Save)
        {
            sceneFilePath = std::string(path) + sceneName + SCENE_EXTENSION;
            fileName      = sceneName;
        }
        else
        {
            fileName      = FileSystem::GetFileNameWithoutExtension(path);
            sceneFilePath = FileSystem::GetFilePath(path) + fileName + SCENE_EXTENSION;
        }

        unsigned int bytesWritten = (unsigned int
        )FileSystem::Save(sceneFilePath.c_str(), buffer.GetString(), (unsigned int)buffer.GetSize(), false);
        if (bytesWritten == 0)
        {
            GLOG("Failed to save scene file: %s", path);
            return false;
        }

        GLOG("%s saved as scene", fileName.c_str());
    }

    return false;
}

bool LibraryModule::LoadScene(const char* path, bool reload) const
{
    rapidjson::Document doc;
    bool loaded = FileSystem::LoadJSON(path, doc);

    if (!loaded)
    {
        GLOG("Failed to load scene file: %s", path);
        return false;
    }

    if (!doc.HasMember("Scene") || !doc["Scene"].IsObject())
    {
        GLOG("Invalid scene format: %s", path);
        return false;
    }

    rapidjson::Value& scene = doc["Scene"];

    App->GetSceneModule()->LoadScene(scene);
    return true;
}

bool LibraryModule::LoadLibraryMaps()
{
    for (const auto& entry : std::filesystem::recursive_directory_iterator(METADATA_PATH))
    {
        if (entry.is_regular_file() && (FileSystem::GetFileExtension(entry.path().string()) == META_EXTENSION))
        {
            std::string filePath = entry.path().string();

            rapidjson::Document doc;
            if (!FileSystem::LoadJSON(filePath.c_str(), doc)) continue;

            UID assetUID          = doc["UID"].GetUint64();
            std::string assetName = doc["name"].GetString();
            std::string assetPath = doc["assetPath"].GetString();

            UID prefix            = assetUID / UID_PREFIX_DIVISOR;
            std::string libraryPath;

            switch (prefix)
            {
            case 11:
                AddMesh(assetUID, assetName);
                AddName(assetName, assetUID);
                libraryPath = MESHES_PATH + std::to_string(assetUID) + MESH_EXTENSION;
                if (FileSystem::Exists(libraryPath.c_str())) AddResource(libraryPath, assetUID);
                else SceneImporter::ImportMeshFromMetadata(assetPath, assetName, assetUID);
                break;
            case 12: 
                AddTexture(assetUID, assetName);
                AddName(assetName, assetUID);
                libraryPath = TEXTURES_PATH + std::to_string(assetUID) + TEXTURE_EXTENSION;
                if (FileSystem::Exists(libraryPath.c_str())) AddResource(libraryPath, assetUID);
                else TextureImporter::Import(assetPath.c_str(), assetUID);
                break;
            case 13:
                AddMaterial(assetUID, assetName);
                AddName(assetName, assetUID);
                libraryPath = MATERIALS_PATH + std::to_string(assetUID) + MATERIAL_EXTENSION;
                if (FileSystem::Exists(libraryPath.c_str())) AddResource(libraryPath, assetUID);
                else SceneImporter::ImportMaterialFromMetadata(assetPath, assetName, assetUID);
                break;
            //case 14:
            //    AddMaterial(assetUID, assetName);
            //    AddName(assetName, assetUID);
            //    libraryPath = MODELS_PATH + std::to_string(assetUID) + MODEL_EXTENSION;
            //    if (FileSystem::Exists(libraryPath.c_str())) AddResource(libraryPath, assetUID);
            //    else SceneImporter::ImportMaterialFromMetadata(assetPath, assetName, assetUID);
            //    break;
            default:
                GLOG("Unknown UID prefix (%s) for: %s", std::to_string(prefix).c_str(), assetName.c_str());
                continue;
            }
        }
    }

    GLOG("MODELS MAP SIZE: %d", modelMap.size());

    return true;
}

UID LibraryModule::AssignFiletypeUID(UID originalUID, FileType fileType)
{
    UID prefix = 10; // Default prefix "99" for unknown files
    switch (fileType)
    {
    case FileType::Mesh:
        prefix = 11;
        break;
    case FileType::Texture:
        prefix = 12;
        break;
    case FileType::Material:
        prefix = 13;
        break;
    case FileType::Model:
        prefix = 14;
        break;
    default:
        GLOG("Category: Unknown File Type (10)");
        break;
    }

    // GLOG("%llu", prefix)
    UID final = (prefix * UID_PREFIX_DIVISOR) + (originalUID % UID_PREFIX_DIVISOR);
    GLOG("%llu", final);
    return final;
}

void LibraryModule::AddTexture(UID textureUID, const std::string& textureName)
{
    textureMap[textureName] = textureUID;
}

void LibraryModule::AddMesh(UID meshUID, const std::string& meshName)
{
    meshMap[meshName] = meshUID;
}

void LibraryModule::AddMaterial(UID materialUID, const std::string& matName)
{
    materialMap[matName] = materialUID;
}

void LibraryModule::AddName(const std::string& resourceName, UID resourceUID)
{
    namesMap[resourceUID] = resourceName;
}

void LibraryModule::AddModel(UID modelUID, const std::string& modelPath)
{
    modelMap[modelPath] = modelUID;
}

UID LibraryModule::GetTextureUID(const std::string& texturePath) const
{
    auto it = textureMap.find(texturePath);
    if (it != textureMap.end())
    {
        return it->second;
    }

    return INVALID_UUID;
}

UID LibraryModule::GetMeshUID(const std::string& meshPath) const
{
    auto it = meshMap.find(meshPath);
    if (it != meshMap.end())
    {
        return it->second;
    }

    return INVALID_UUID;
}

UID LibraryModule::GetMaterialUID(const std::string& materialPath) const
{
    auto it = materialMap.find(materialPath);
    if (it != materialMap.end())
    {
        return it->second;
    }

    return INVALID_UUID;
}

UID LibraryModule::GetModelUID(const std::string& modelPath) const
{
    auto it = materialMap.find(modelPath);
    if (it != materialMap.end())
    {
        return it->second;
    }

    return INVALID_UUID;
}

const std::string& LibraryModule::GetResourcePath(UID resourceID) const
{
    auto it = resourcePathsMap.find(resourceID);
    if (it != resourcePathsMap.end())
    {
        GLOG("requested uid: %llu", resourceID);
        GLOG("obtained path: %s", it->second.c_str());
        return it->second;
    }
    static const std::string emptyString = "";
    return emptyString;
}

const std::string& LibraryModule::GetResourceName(UID resourceID) const
{
    auto it = namesMap.find(resourceID);
    if (it != namesMap.end())
    {
        GLOG("requested uid: %llu", resourceID);
        GLOG("obtained name: %s", it->second.c_str());
        return it->second;
    }
    static const std::string emptyString = "";
    return emptyString;
}

void LibraryModule::AddResource(const std::string& resourcePath, UID resourceUID)
{
    resourcePathsMap[resourceUID] = resourcePath;
}