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
    void Render() override;

    const ResourceMesh* GetResourceMesh() const { return currentMesh; }

    void AddMesh(UID resource, bool updateParent = true);
    void AddMaterial(UID resource);

    void SetBones(const std::vector<GameObject*>& bones) { this->bones = bones; }
    void SetBindTransforms(const std::vector<float4x4>& bindTransforms) { this->bindTransforms = bindTransforms; }

  private:
    std::string currentMeshName       = "Not selected";
    ResourceMesh* currentMesh         = nullptr;

    std::string currentMaterialName   = "Not selected";
    ResourceMaterial* currentMaterial = nullptr;

    std::vector<GameObject*> bones;
    std::vector<float4x4> bindTransforms;
};
