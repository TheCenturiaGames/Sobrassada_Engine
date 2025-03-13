#pragma once

#include "Globals.h"
#include "Module.h"

#include <string>
#include <unordered_map>

enum class SaveMode
{
    Save,
    SaveAs,
    SavePlayMode
};

enum class FileType
{
    Mesh,
    Texture,
    Material,
    Scene,
    Model
};

class LibraryModule : public Module
{

  public:
    LibraryModule();
    ~LibraryModule() override;

    bool Init() override;

    bool SaveScene(const char* path, SaveMode saveMode) const;
    bool LoadScene(const char* fileName, bool reload = false) const;

    bool LoadLibraryMaps(const std::string& projectPath);
    UID AssignFiletypeUID(UID originalUID, FileType fileType);

    void AddTexture(UID textureUID, const std::string& ddsPath);
    void AddMesh(UID meshUID, const std::string& matPath);
    void AddMaterial(UID materialUID, const std::string& sobPath);
    void AddModel(UID modelUID, const std::string& modelPath);
    void AddName(const std::string& resourceName, UID resourceUID);
    void AddResource(const std::string& resourcePath, UID resourceUID);

    UID GetTextureUID(const std::string& texturePath) const;
    UID GetMeshUID(const std::string& meshPath) const;
    UID GetMaterialUID(const std::string& materialPath) const;
    UID GetModelUID(const std::string& modelPath) const;

    const std::string& GetResourceName(UID resourceID) const;

    const std::string& GetResourcePath(UID resourceID) const;

    const std::unordered_map<std::string, UID>& GetTextureMap() const { return textureMap; }
    const std::unordered_map<std::string, UID>& GetMaterialMap() const { return materialMap; }
    const std::unordered_map<std::string, UID>& GetMeshMap() const { return meshMap; }
    const std::unordered_map<std::string, UID>& GetModelMap() const { return modelMap; }

  private:
    // maps for user visuals | name -> UID
    std::unordered_map<std::string, UID> textureMap;
    std::unordered_map<std::string, UID> materialMap;
    std::unordered_map<std::string, UID> meshMap;
    std::unordered_map<std::string, UID> modelMap;
    // inversed map          | UID -> name
    std::unordered_map<UID, std::string> namesMap;

    // filled on load and import
    std::unordered_map<UID, std::string> resourcePathsMap; // UID -> library path
};
