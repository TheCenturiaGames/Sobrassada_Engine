﻿#pragma once

#include "Globals.h"
#include "ResourceManagement/Resources/ResourceMaterial.h"
#include "ResourceManagement/Resources/ResourceMesh.h"
#include "Scene/Components/Component.h"

#include <Libs/rapidjson/document.h>
#include <cstdint>

class MeshComponent : public Component
{
  public:
    MeshComponent(UID uid, UID uidParent, UID uidRoot, const float4x4& parentGlobalTransform);

    MeshComponent(const rapidjson::Value& initialState);

    virtual void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const;

    void RenderEditorInspector() override;
    void Update() override;
    void Render() override;
    void RenderEditorInspector() override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;

  private:
    void AddMesh(UID resource, bool reloadAABB = true);
    void AddMaterial(UID resource);

  private:
    std::string currentMeshName       = "Not selected";
    ResourceMesh* currentMesh         = nullptr;

    std::string currentTextureName    = "Not selected";
    ResourceMaterial* currentMaterial = nullptr;
};
