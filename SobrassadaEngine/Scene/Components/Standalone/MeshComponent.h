#pragma once

#include "Globals.h"
#include "ResourceManagement/Resources/ResourceMaterial.h"
#include "ResourceManagement/Resources/ResourceMesh.h"
#include "Scene/Components/Component.h"

#include "Math/float4x4.h"
#include <Libs/rapidjson/document.h>
#include <cstdint>

class MeshComponent : public Component
{
  public:
    MeshComponent(UID uid, UID uidParent);

    MeshComponent(const rapidjson::Value& initialState);

    ~MeshComponent() override;

    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;

    void RenderEditorInspector() override;
    void Update() override;
    void Render(float deltaTime) override;
    bool HasBones() const { return bones.size() > 0; }

    void InitSkin();

    const ResourceMesh* GetResourceMesh() const { return currentMesh; }
    
    void AddMesh(UID resource, bool updateParent = true);
    void AddMaterial(UID resource);

    void SetBones(const std::vector<GameObject*>& bones, const std::vector<UID> bonesIds)
    {
        this->bones     = bones;
        this->bonesUIDs = bonesIds;
    }
    void SetBindMatrices(const std::vector<float4x4>& bindTransforms) { this->bindMatrices = bindTransforms; }
    void SetModelUID(const UID newModelUID) { this->modelUID = newModelUID; }
    void SetSkinIndex(const int newIndex) { this->skinIndex = newIndex; }

  private:

  private:
    std::string currentMeshName       = "Not selected";
    ResourceMesh* currentMesh         = nullptr;

    std::string currentMaterialName   = "Not selected";
    ResourceMaterial* currentMaterial = nullptr;

    std::vector<UID> bonesUIDs;
    std::vector<GameObject*> bones;
    std::vector<float4x4> bindMatrices;

    UID modelUID  = INVALID_UID;
    int skinIndex = -1;
};
