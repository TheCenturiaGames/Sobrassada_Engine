#include "LibraryModule.h"

#include "Application.h"
#include "Component.h"
#include "ComponentUtils.h"
#include "FileSystem.h"
#include "GameObject.h"
#include "Root/RootComponent.h"
#include "SceneImporter.h"
#include "SceneModule.h"

#include "document.h"
#include "prettywriter.h"
#include "stringbuffer.h"
#include <filesystem>

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

// Save         = Path: library/scenes/   | UID: sceneUID     | FileName: scene name
// SaveAs       = Path: fileName          | UID: generate new | FileName: path filename
// SavePlayMode = Path: sceneUID          | UID: sceneUID     | FileName: sceneUID
bool LibraryModule::SaveScene(const char* path, SaveMode saveMode) const
{
    SceneModule* sceneModule = App->GetSceneModule();
    if (sceneModule == nullptr || !sceneModule->IsSceneLoaded())
    {
        GLOG("Scene not found");
        return false;
    }

    UID sceneUID = CONSTANT_EMPTY_UID;
    std::string sceneName;
    switch (saveMode)
    {
    case SaveMode::Save:
        sceneUID  = sceneModule->GetSceneUID();
        sceneName = sceneModule->GetSceneName();
        break;
    case SaveMode::SaveAs:
        sceneUID  = GenerateUID();
        sceneName = std::string(path);
        break;
    case SaveMode::SavePlayMode:
        sceneUID  = sceneModule->GetSceneUID();
        sceneName = sceneModule->GetSceneName();
        break;
    }

    if (sceneUID == CONSTANT_EMPTY_UID && saveMode == SaveMode::Save) return false;

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
    else
    {
        GLOG("Light Config not found");
        return false;
    }

    // Save file like JSON
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    std::string sceneFilePath;
    if (saveMode == SaveMode::SavePlayMode) sceneFilePath = SCENES_PATH + std::to_string(sceneUID) + SCENE_EXTENSION;
    else sceneFilePath = SCENES_PATH + sceneName + SCENE_EXTENSION;

    unsigned int bytesWritten = (unsigned int
    )FileSystem::Save(sceneFilePath.c_str(), buffer.GetString(), (unsigned int)buffer.GetSize(), false);
    if (bytesWritten == 0)
    {
        GLOG("Failed to save scene file: %s", sceneName.c_str());
        return false;
    }

    GLOG("%s saved as scene", sceneName.c_str());
    return true;
}

bool LibraryModule::LoadScene(const char* file, bool reload) const
{
    rapidjson::Document doc;
    bool loaded = FileSystem::LoadJSON((SCENES_PATH + std::string(file)).c_str(), doc);

    if (!loaded)
    {
        GLOG("Failed to load scene file: %s", file);
        return false;
    }

    if (!doc.HasMember("Scene") || !doc["Scene"].IsObject())
    {
        GLOG("Invalid scene format: %s", file);
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

    for (const auto& entry : std::filesystem::recursive_directory_iterator(LIBRARY_PATH))
    {

        if (entry.is_regular_file())
        {
            std::string filePath = entry.path().string();

            std::string fileName = FileSystem::GetFileNameWithoutExtension(filePath);

            // Generate UID using the function from globals.h
            try
            {
                UID originalUID = std::stoull(fileName);

                UID prefix      = originalUID / 100000000000000;

                switch (prefix)
                {
                case 13:
                    AddMesh(originalUID, FileSystem::GetFileNameWithoutExtension(filePath));
                    AddResource(filePath, originalUID);
                    break;
                case 12:
                    AddMaterial(originalUID, FileSystem::GetFileNameWithoutExtension(filePath));
                    AddResource(filePath, originalUID);
                    break;
                case 11:
                    AddTexture(originalUID, FileSystem::GetFileNameWithoutExtension(filePath));
                    AddResource(filePath, originalUID);
                    break;
                default:
                    GLOG("Category: Unknown File Type (10)");
                    break;
                }
            }
            catch (std::invalid_argument&)
            {
                GLOG("File %s is not an asset file", fileName);
            }
        }
    }

    return true;
}

UID LibraryModule::AssignFiletypeUID(UID originalUID, const std::string& filePath)
{

    uint64_t prefix = 10; // Default prefix "99" for unknown files
    if (FileSystem::GetFileExtension(filePath) == MESH_EXTENSION)
    {
        prefix = 13;
    }
    if (FileSystem::GetFileExtension(filePath) == MATERIAL_EXTENSION)
    {
        prefix = 12;
    }
    if (FileSystem::GetFileExtension(filePath) == TEXTURE_EXTENSION)
    {
        prefix = 11;
    }
    // GLOG("%llu", prefix)
    uint64_t final = (prefix * 100000000000000) + (originalUID % 100000000000000);
    GLOG("%llu", final);
    return final;
}

void LibraryModule::AddTexture(UID textureUID, const std::string& ddsPath)
{
    textureMap[ddsPath] = textureUID; // Map the texture UID to its DDS path
}

void LibraryModule::AddMesh(UID meshUID, const std::string& sobPath)
{
    meshMap[sobPath] = meshUID; // Map the texture UID to its DDS path
}

void LibraryModule::AddMaterial(UID materialUID, const std::string& matPath)
{
    materialMap[matPath] = materialUID; // Map the texture UID to its DDS path
}

UID LibraryModule::GetTextureUID(const std::string& texturePath) const
{

    auto it = textureMap.find(texturePath);
    if (it != textureMap.end())
    {
        return it->second;
    }

    return 0;
}

UID LibraryModule::GetMeshUID(const std::string& meshPath) const
{

    auto it = meshMap.find(meshPath);
    if (it != meshMap.end())
    {
        return it->second;
    }

    return 0;
}

UID LibraryModule::GetMaterialUID(const std::string& materialPath) const
{

    auto it = materialMap.find(materialPath);
    if (it != materialMap.end())
    {
        return it->second;
    }

    return 0;
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

void LibraryModule::AddResource(const std::string& resourcePath, UID resourceUID)
{
    resourcePathsMap[resourceUID] = resourcePath;
}
