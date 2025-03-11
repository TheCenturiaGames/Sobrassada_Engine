#include "MeshComponent.h"

#include "Application.h"
#include "CameraModule.h"
#include "EditorUIModule.h"
#include "FileSystem/MeshImporter.h"
#include "LibraryModule.h"
#include "ResourcesModule.h"
// #include "Scene/GameObjects/GameObject.h"

#include "imgui.h"

#include <Math/Quat.h>

MeshComponent::MeshComponent(const UID uid, const UID uidParent) : Component(uid, uidParent, "Mesh", COMPONENT_MESH)
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
        AddMesh(initialState["Mesh"].GetUint64(), false);
    }
}

MeshComponent::~MeshComponent()
{
    App->GetResourcesModule()->ReleaseResource(currentMaterial);
    App->GetResourcesModule()->ReleaseResource(currentMesh);
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
            AddMesh(App->GetEditorUIModule()->RenderResourceSelectDialog<UID>(
                CONSTANT_MESH_SELECT_DIALOG_ID, App->GetLibraryModule()->GetMeshMap(), INVALID_UUID
            ));
        }

        ImGui::SeparatorText("Material");
        ImGui::Text(currentMaterialName.c_str());
        ImGui::SameLine();
        if (ImGui::Button("Select material"))
        {
            ImGui::OpenPopup(CONSTANT_TEXTURE_SELECT_DIALOG_ID);
        }

        if (ImGui::IsPopupOpen(CONSTANT_TEXTURE_SELECT_DIALOG_ID))
        {
            AddMaterial(App->GetEditorUIModule()->RenderResourceSelectDialog<UID>(
                CONSTANT_TEXTURE_SELECT_DIALOG_ID, App->GetLibraryModule()->GetMaterialMap(), INVALID_UUID
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
        int program            = App->GetResourcesModule()->GetProgram();
        if (currentMaterial != nullptr)
        {
            if(!currentMaterial->GetIsMetallicRoughness()) program = App->GetResourcesModule()->GetSpecularProgram();
        }
        currentMesh->Render(program, GetParent()->GetGlobalTransform(), cameraUBO, currentMaterial);
    }
}

void MeshComponent::AddMesh(UID resource, bool updateParent)
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
        GameObject* parent = GetParent();
        if (parent != nullptr)
        {
            parent->OnAABBUpdated();
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
        currentMaterial     = newMaterial;
        currentMaterialName = currentMaterial->GetName();
    }
}