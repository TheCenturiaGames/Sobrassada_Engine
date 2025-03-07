#include "LibraryModule.h"

#include "Application.h"
#include "Component.h"
#include "ComponentUtils.h"
#include "FileSystem.h"
#include "GameObject.h"
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

// Save = library/scenes/
// SaveAs = all path + name.scene
bool LibraryModule::SaveScene(const char* path, SaveMode saveMode) const
{
    Scene* loadedScene = App->GetSceneModule()->GetScene();
    if (loadedScene != nullptr)
    {
        // Create doc JSON
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        rapidjson::Value scene(rapidjson::kObjectType);

        const char* sceneName = loadedScene->Save(scene, allocator);

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
