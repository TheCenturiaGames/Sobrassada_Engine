#include "LibraryModule.h"

#include "Application.h"
#include "Component.h"
#include "ComponentUtils.h"
#include "FileSystem.h"
#include "GameObject.h"
#include "Root/RootComponent.h"
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
    SceneModule* sceneModule = App->GetSceneModule();
    if (sceneModule == nullptr)
    {
        GLOG("Scene module not found");
        return false;
    }

    UID sceneUID = (saveMode == SaveMode::Save) ? sceneModule->GetSceneUID() : GenerateUID();
    const std::string& sceneName =
        (saveMode == SaveMode::Save) ? sceneModule->GetSceneName() : FileSystem::GetFileNameWithoutExtension(path);

    if (sceneName == DEFAULT_SCENE_NAME && saveMode == SaveMode::Save) return false;

    UID gameObjectRootUID   = sceneModule->GetGameObjectRootUID();

    const auto* gameObjects = sceneModule->GetAllGameObjects();
    const auto* components  = sceneModule->GetAllComponents();

    // Create doc JSON
    rapidjson::Document doc;
    doc.SetObject();
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

    rapidjson::Value scene(rapidjson::kObjectType);

    // Scene values
    UID uid                 = sceneUID;
    const std::string& name = sceneName;
    UID rootGameObject      = gameObjectRootUID;

    // Create structure
    scene.AddMember("UID", uid, allocator);
    scene.AddMember("Name", rapidjson::Value(name.c_str(), allocator), allocator);
    scene.AddMember("RootGameObject", rootGameObject, allocator);

    // Serialize GameObjects
    rapidjson::Value gameObjectsJSON(rapidjson::kArrayType);

    for (auto it = gameObjects->begin(); it != gameObjects->end(); ++it)
    {
        if (it->second != nullptr)
        {
            rapidjson::Value goJSON(rapidjson::kObjectType);

            it->second->Save(goJSON, allocator);

            gameObjectsJSON.PushBack(goJSON, allocator);
        }
    }

    // Add gameObjects to scene
    scene.AddMember("GameObjects", gameObjectsJSON, allocator);

    // Serialize Components
    rapidjson::Value componentsJSON(rapidjson::kArrayType);

    for (auto it = components->begin(); it != components->end(); ++it)
    {
        if (it->second != nullptr)
        {
            rapidjson::Value componentJSON(rapidjson::kObjectType);

            it->second->Save(componentJSON, allocator);

            componentsJSON.PushBack(componentJSON, allocator);
        }
    }

    // Add components to scene
    scene.AddMember("Components", componentsJSON, allocator);

    doc.AddMember("Scene", scene, allocator);

    // Serialize Lights Config
    LightsConfig* lightConfig = App->GetSceneModule()->GetLightsConfig();

    if (lightConfig != nullptr)
    {
        rapidjson::Value lights(rapidjson::kObjectType);

        lightConfig->SaveData(lights, allocator);

        doc.AddMember("Lights Config", lights, allocator);
    }
    else GLOG("Light Config not found");

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

    return true;
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

    // Scene values
    UID sceneUID            = scene["UID"].GetUint64();
    std::string name        = scene["Name"].GetString();
    UID rootGameObject      = scene["RootGameObject"].GetUint64();

    if (sceneUID == App->GetSceneModule()->GetSceneUID() && !reload)
    {
        GLOG("Scene already loaded: %s", name.c_str());
        return false;
    }

    App->GetSceneModule()->CloseScene();

    std::map<UID, Component*> loadedGameComponents;
    std::unordered_map<UID, GameObject*> loadedGameObjects;

    // Deserialize Components
    if (scene.HasMember("Components") && scene["Components"].IsArray())
    {
        const rapidjson::Value& components = scene["Components"];

        for (rapidjson::SizeType i = 0; i < components.Size(); i++)
        {
            const rapidjson::Value& component = components[i];

            Component* newComponent           = ComponentUtils::CreateExistingComponent(component);

            if (newComponent != nullptr)
            {
                loadedGameComponents.insert({newComponent->GetUID(), newComponent});
            }
        }
    }

    App->GetSceneModule()->LoadScene(sceneUID, name.c_str(), rootGameObject, loadedGameComponents);

    // Deserialize GameObjects
    if (scene.HasMember("GameObjects") && scene["GameObjects"].IsArray())
    {
        const rapidjson::Value& gameObjects = scene["GameObjects"];
        for (rapidjson::SizeType i = 0; i < gameObjects.Size(); i++)
        {
            const rapidjson::Value& gameObject = gameObjects[i];

            GameObject* newGameObject          = new GameObject(gameObject);

            loadedGameObjects.insert({newGameObject->GetUID(), newGameObject});
        }
    }

    App->GetSceneModule()->LoadGameObjects(loadedGameObjects);

    // Deserialize Lights Config
    if (doc.HasMember("Lights Config") && doc["Lights Config"].IsObject())
    {
        LightsConfig* lightConfig = App->GetSceneModule()->GetLightsConfig();
        lightConfig->LoadData(doc["Lights Config"]);
    }

    GLOG("%s scene loaded", name.c_str());
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