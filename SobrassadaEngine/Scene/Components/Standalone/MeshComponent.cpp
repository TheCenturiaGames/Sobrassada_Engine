﻿#include "MeshComponent.h"

#include "../Root/RootComponent.h"
#include "Application.h"
#include "CameraModule.h"
#include "EditorUIModule.h"
#include "FileSystem/MeshImporter.h"
#include "LibraryModule.h"
#include "ResourcesModule.h"
#include "SceneModule.h"

#include "imgui.h"
#include <Math/Quat.h>

MeshComponent::MeshComponent(
    const UID uid, const UID uidParent, const UID uidRoot, const float4x4& parentGlobalTransform
)
    : Component(uid, uidParent, uidRoot, "Mesh", COMPONENT_MESH, parentGlobalTransform)
{
}

MeshComponent::MeshComponent(const rapidjson::Value& initialState) : Component(initialState)
{
    if (initialState.HasMember("Material"))
    {
        AddMaterial(initialState["Material"].GetUint64());
    }
    if (initialState.HasMember("Mesh"))
    {
        AddMesh(initialState["Mesh"].GetUint64(), false); // Do not update aabb, will be done once at the end
    }
}

void MeshComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);

    targetState.AddMember("Mesh", currentMesh != nullptr ? currentMesh->GetUID() : CONSTANT_EMPTY_UID, allocator);
    targetState.AddMember(
        "Material", currentMaterial != nullptr ? currentMaterial->GetUID() : CONSTANT_EMPTY_UID, allocator
    );
}

void MeshComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();

    if (enabled)
    {
        ImGui::SeparatorText("Mesh");
        ImGui::Text(currentMeshName.c_str());
        ImGui::SameLine();
        if (ImGui::Button("Select mesh"))
        {
            ImGui::OpenPopup(CONSTANT_MESH_SELECT_DIALOG_ID);
        }

        if (ImGui::IsPopupOpen(CONSTANT_MESH_SELECT_DIALOG_ID))
        {
            AddMesh(App->GetEditorUIModule()->RenderResourceSelectDialog(
                CONSTANT_MESH_SELECT_DIALOG_ID, App->GetLibraryModule()->GetMeshMap()
            ));
        }

        ImGui::SeparatorText("Material");
        ImGui::Text(currentTextureName.c_str());
        ImGui::SameLine();
        if (ImGui::Button("Select texture"))
        {
            ImGui::OpenPopup(CONSTANT_TEXTURE_SELECT_DIALOG_ID);
        }

        if (ImGui::IsPopupOpen(CONSTANT_TEXTURE_SELECT_DIALOG_ID))
        {
            AddMaterial(App->GetEditorUIModule()->RenderResourceSelectDialog(
                CONSTANT_TEXTURE_SELECT_DIALOG_ID, App->GetLibraryModule()->GetMaterialMap()
            ));
        }

        if (currentMaterial != nullptr) currentMaterial->OnEditorUpdate();
    }
}

void MeshComponent::Update()
{
}

void MeshComponent::Render()
{
    if (enabled && currentMesh != nullptr)
    {
        unsigned int cameraUBO = App->GetCameraModule()->GetUbo();
        currentMesh->Render(App->GetResourcesModule()->GetProgram(), globalTransform, cameraUBO, currentMaterial);
    }
    Component::Render();
}

void MeshComponent::AddMesh(UID resource, bool reloadAABB)
{
    if (resource == CONSTANT_EMPTY_UID) return;

    if (currentMesh != nullptr && currentMesh->GetUID() == resource) return;

    ResourceMesh* newMesh = dynamic_cast<ResourceMesh*>(App->GetResourcesModule()->RequestResource(resource));
    if (newMesh != nullptr)
    {
        App->GetResourcesModule()->ReleaseResource(currentMesh);
        currentMeshName    = newMesh->GetName();
        currentMesh        = newMesh;
        localComponentAABB = AABB(currentMesh->GetAABB());

        if (reloadAABB)
        {
            globalComponentAABB   = AABB(currentMesh->GetAABB());
            AABBUpdatable* parent = GetParent();
            if (parent != nullptr)
            {
                parent->PassAABBUpdateToParent();
            }
        }
    }
}

void MeshComponent::AddMaterial(UID resource)
{
    if (currentMaterial != nullptr && currentMaterial->GetUID() == resource) return;

    ResourceMaterial* newMaterial =
        dynamic_cast<ResourceMaterial*>(App->GetResourcesModule()->RequestResource(resource));
    if (newMaterial != nullptr)
    {
        App->GetResourcesModule()->ReleaseResource(currentMaterial);
        currentMaterial    = newMaterial;
        currentTextureName = currentMaterial->GetName();
    }
}