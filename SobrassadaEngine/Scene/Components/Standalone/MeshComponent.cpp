#include "MeshComponent.h"

#include "Application.h"
#include "CameraModule.h"
#include "EditorUIModule.h"
#include "FileSystem/MeshImporter.h"
#include "LibraryModule.h"
#include "ResourceManagement/Resources/ResourceModel.h"
#include "ResourcesModule.h"
#include "SceneModule.h"
#include "ShaderModule.h"
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
    if (initialState.HasMember("Bones"))
    {
        const rapidjson::Value& initBones = initialState["Bones"];
        for (unsigned int i = 0; i < initBones.Size(); ++i)
        {
            bonesUIDs.push_back(initBones[i].GetUint64());
        }
    }
    if (initialState.HasMember("ModelUID"))
    {
        modelUID = initialState["ModelUID"].GetUint64();

        // Get the skin index and the bind matrices
        if (initialState.HasMember("SkinIndex"))
        {
            ResourceModel* model = static_cast<ResourceModel*>(App->GetResourcesModule()->RequestResource(modelUID));
            skinIndex            = initialState["SkinIndex"].GetInt();
            bindMatrices         = model->GetModelData().GetSkin(skinIndex).inverseBindMatrices;
        }
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

    targetState.AddMember("Mesh", currentMesh != nullptr ? currentMesh->GetUID() : INVALID_UID, allocator);
    targetState.AddMember("Material", currentMaterial != nullptr ? currentMaterial->GetUID() : INVALID_UID, allocator);

    if (bones.size() > 0) // Store the skin of the mesh as the UID of each bone
    {
        rapidjson::Value valBones(rapidjson::kArrayType);
        for (const UID boneUID : bonesUIDs)
        {
            valBones.PushBack(boneUID, allocator);
        }

        targetState.AddMember("Bones", valBones, allocator);

        // Save the resource model UID used to load this mesh, so it can get the bind matrices when loading
        if (modelUID != INVALID_UID) targetState.AddMember("ModelUID", modelUID, allocator);
        if (skinIndex != -1) targetState.AddMember("SkinIndex", skinIndex, allocator);
    }
}

void MeshComponent::Clone(const Component* other)
{
    if (other->GetType() == ComponentType::COMPONENT_MESH)
    {
        const MeshComponent* otherMesh = static_cast<const MeshComponent*>(other);
        enabled                        = otherMesh->enabled;

        AddMesh(otherMesh->currentMesh->GetUID());
        AddMaterial(otherMesh->currentMaterial->GetUID());

        modelUID     = otherMesh->modelUID;
        skinIndex    = otherMesh->skinIndex;
        bindMatrices = otherMesh->bindMatrices;
    }
    else
    {
        GLOG("It is not possible to clone a component of a different type!");
    }
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
                CONSTANT_MESH_SELECT_DIALOG_ID, App->GetLibraryModule()->GetMeshMap(), INVALID_UID
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
                CONSTANT_TEXTURE_SELECT_DIALOG_ID, App->GetLibraryModule()->GetMaterialMap(), INVALID_UID
            ));
        }

        if (currentMaterial != nullptr) currentMaterial->OnEditorUpdate();
    }
}

void MeshComponent::Update()
{
}

void MeshComponent::Render(float deltaTime)
{
    if (enabled && currentMesh != nullptr)
    {
        unsigned int cameraUBO = App->GetCameraModule()->GetUbo();
        int program            = App->GetShaderModule()->GetMetallicRoughnessProgram();

        if (currentMaterial != nullptr)
        {
            if (!currentMaterial->GetIsMetallicRoughness())
                program = App->GetShaderModule()->GetSpecularGlossinessProgram();
        }
        if (App->GetSceneModule()->GetInPlayMode() && App->GetSceneModule()->GetMainCamera() != nullptr)
            cameraUBO = App->GetSceneModule()->GetMainCamera()->GetUbo();

        currentMesh->Render(
            program, GetParent()->GetGlobalTransform(), cameraUBO, currentMaterial, bones, bindMatrices
        );
    }
}

void MeshComponent::InitSkin()
{
    if (bonesUIDs.size() > 0) return;
    for (UID uid : bonesUIDs)
    {
        bones.emplace_back(App->GetSceneModule()->GetGameObjectByUID(uid));
    }
}

void MeshComponent::AddMesh(UID resource, bool updateParent)
{
    if (resource == INVALID_UID) return;

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