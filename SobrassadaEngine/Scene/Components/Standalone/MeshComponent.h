﻿#pragma once

#include "Component.h"
#include "Globals.h"

#include <Libs/rapidjson/document.h>
#include <Math/float4x4.h>
#include <cstdint>

class ResourceMesh;
class ResourceMaterial;
class GeometryBatch;

class MeshComponent : public Component
{
  public:
    MeshComponent(UID uid, GameObject* parent);

    MeshComponent(const rapidjson::Value& initialState, GameObject* parent);

    ~MeshComponent() override;

    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void Clone(const Component* other) override;

    void RenderEditorInspector() override;

    void Update(float deltaTime) override;
    void Render(float deltaTime) override;

    bool HasBones() const { return bones.size() > 0; }

    void InitSkin();

    const ResourceMesh* GetResourceMesh() const { return currentMesh; }
    const ResourceMaterial* GetResourceMaterial() const { return currentMaterial; }

    void AddMesh(UID resource, bool updateParent = true);
    void AddMaterial(UID resource);

    const bool GetHasBones() const { return hasBones; }
    const std::vector<GameObject*>& GetBonesGO() const { return bones; }
    const std::vector<UID>& GetBones() const { return bonesUIDs; }
    const std::vector<float4x4>& GetBindMatrices() const { return bindMatrices; }
    void SetBones(const std::vector<GameObject*>& bones, const std::vector<UID> bonesIds)
    {
        this->bones     = bones;
        this->bonesUIDs = bonesIds;
    }
    void SetBindMatrices(const std::vector<float4x4>& bindTransforms) { this->bindMatrices = bindTransforms; }
    void SetModelUID(const UID newModelUID) { this->modelUID = newModelUID; }
    void SetSkinIndex(const int newIndex) { this->skinIndex = newIndex; }

    void OnTransformUpdated();

    const float4x4& GetCombinedMatrix() const { return combinedMatrix; }

    GeometryBatch* GetBatch() const { return batch; }

  private:
    std::string currentMeshName       = "Not selected";
    ResourceMesh* currentMesh         = nullptr;

    std::string currentMaterialName   = "Not selected";
    ResourceMaterial* currentMaterial = nullptr;

    std::vector<UID> bonesUIDs;
    std::vector<GameObject*> bones;
    std::vector<float4x4> bindMatrices;
    bool hasBones           = false;

    UID modelUID            = INVALID_UID;
    int skinIndex           = -1;

    float4x4 combinedMatrix = float4x4::identity;

    GeometryBatch* batch    = nullptr;
    bool editorMesh         = false;
};
