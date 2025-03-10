#include "GameObject.h"

#include "Application.h"
#include "Component.h"
#include "DebugDrawModule.h"
#include "EditorUIModule.h"
#include "SceneModule.h"

#include "Standalone/MeshComponent.h"
#include "imgui.h"

#include <stack>

GameObject::GameObject(std::string name) : name(name)
{
    uid        = GenerateUID();
    parentUID  = INVALID_UUID;
    localAABB  = AABB(DEFAULT_GAME_OBJECT_AABB);
    globalOBB  = OBB(localAABB);
    globalAABB = AABB(globalOBB);
}

GameObject::GameObject(UID parentUUID, std::string name) : parentUID(parentUUID), name(name)
{
    uid        = GenerateUID();
    localAABB  = AABB(DEFAULT_GAME_OBJECT_AABB);
    globalOBB  = OBB(localAABB);
    globalAABB = AABB(globalOBB);
}

GameObject::GameObject(const rapidjson::Value& initialState) : uid(initialState["UID"].GetUint64())
{
    parentUID              = initialState["ParentUID"].GetUint64();
    name                   = initialState["Name"].GetString();
    selectedComponentIndex = COMPONENT_NONE;
    mobilitySettings       = initialState["Mobility"].GetInt();

    if (initialState.HasMember("LocalTransform") && initialState["LocalTransform"].IsArray() &&
        initialState["LocalTransform"].Size() == 16)
    {
        const rapidjson::Value& initLocalTransform = initialState["LocalTransform"];

        localTransform                             = float4x4(
            initLocalTransform[0].GetFloat(), initLocalTransform[1].GetFloat(), initLocalTransform[2].GetFloat(),
            initLocalTransform[3].GetFloat(), initLocalTransform[4].GetFloat(), initLocalTransform[5].GetFloat(),
            initLocalTransform[6].GetFloat(), initLocalTransform[7].GetFloat(), initLocalTransform[8].GetFloat(),
            initLocalTransform[9].GetFloat(), initLocalTransform[10].GetFloat(), initLocalTransform[11].GetFloat(),
            initLocalTransform[12].GetFloat(), initLocalTransform[13].GetFloat(), initLocalTransform[14].GetFloat(),
            initLocalTransform[15].GetFloat()
        );
    }

    // Deserialize Components
    if (initialState.HasMember("Components") && initialState["Components"].IsArray())
    {
        const rapidjson::Value& jsonComponents = initialState["Components"];

        for (rapidjson::SizeType i = 0; i < jsonComponents.Size(); i++)
        {
            const rapidjson::Value& jsonComponent = jsonComponents[i];

            Component* newComponent               = ComponentUtils::CreateExistingComponent(jsonComponent);

            if (newComponent != nullptr)
            {
                components.insert({newComponent->GetType(), newComponent});
            }
        }
    }

    OnAABBUpdated();

    if (initialState.HasMember("Children") && initialState["Children"].IsArray())
    {
        const rapidjson::Value& initChildren = initialState["Children"];

        for (rapidjson::SizeType i = 0; i < initChildren.Size(); i++)
        {
            children.push_back(initChildren[i].GetUint64());
        }
    }
}

GameObject::~GameObject()
{
    for (auto& component : components)
    {
        delete component.second;
    }
    components.clear();
}

bool GameObject::AddGameObject(UID gameObjectUUID)
{
    if (std::find(children.begin(), children.end(), gameObjectUUID) == children.end())
    {
        children.push_back(gameObjectUUID);
        return true;
    }
    return false;
}

bool GameObject::RemoveGameObject(UID gameObjectUUID)
{
    if (const auto it = std::find(children.begin(), children.end(), gameObjectUUID); it != children.end())
    {
        children.erase(it);
        return true;
    }
    return false;
}

void GameObject::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    targetState.AddMember("UID", uid, allocator);
    targetState.AddMember("ParentUID", parentUID, allocator);
    targetState.AddMember("Name", rapidjson::Value(name.c_str(), allocator), allocator);

    targetState.AddMember("Mobility", mobilitySettings, allocator);
    rapidjson::Value valLocalTransform(rapidjson::kArrayType);
    valLocalTransform.PushBack(localTransform.ptr()[0], allocator)
        .PushBack(localTransform.ptr()[1], allocator)
        .PushBack(localTransform.ptr()[2], allocator)
        .PushBack(localTransform.ptr()[3], allocator)
        .PushBack(localTransform.ptr()[4], allocator)
        .PushBack(localTransform.ptr()[5], allocator)
        .PushBack(localTransform.ptr()[6], allocator)
        .PushBack(localTransform.ptr()[7], allocator)
        .PushBack(localTransform.ptr()[8], allocator)
        .PushBack(localTransform.ptr()[9], allocator)
        .PushBack(localTransform.ptr()[10], allocator)
        .PushBack(localTransform.ptr()[11], allocator)
        .PushBack(localTransform.ptr()[12], allocator)
        .PushBack(localTransform.ptr()[13], allocator)
        .PushBack(localTransform.ptr()[14], allocator)
        .PushBack(localTransform.ptr()[15], allocator);

    targetState.AddMember("LocalTransform", valLocalTransform, allocator);

    // Serialize Components
    rapidjson::Value componentsJSON(rapidjson::kArrayType);

    for (auto it = components.begin(); it != components.end(); ++it)
    {
        if (it->second != nullptr)
        {
            rapidjson::Value componentJSON(rapidjson::kObjectType);

            it->second->Save(componentJSON, allocator);

            componentsJSON.PushBack(componentJSON, allocator);
        }
    }

    // Add components to scene
    targetState.AddMember("Components", componentsJSON, allocator);

    rapidjson::Value valChildren(rapidjson::kArrayType);

    for (const UID child : children)
    {
        valChildren.PushBack(child, allocator);
    }

    targetState.AddMember("Children", valChildren, allocator);
}

void GameObject::RenderEditorInspector()
{
    if (!ImGui::Begin("Inspector", &App->GetEditorUIModule()->inspectorMenu))
    {
        ImGui::End();
        return;
    }

    ImGui::Text(name.c_str());

    if (uid != App->GetSceneModule()->GetGameObjectRootUID())
    {
        if (ImGui::Button("Add Component"))
        {
            ImGui::OpenPopup("ComponentSelection");
        }

        auto selectedType = App->GetEditorUIModule()->RenderResourceSelectDialog<ComponentType>(
            "ComponentSelection", standaloneComponents, COMPONENT_NONE
        );
        if (selectedType != COMPONENT_NONE)
        {
            CreateComponent(selectedType);
        }

        const float4x4& parentTransform = GetParentGlobalTransform();

        if (selectedComponentIndex != COMPONENT_NONE)
        {
            ImGui::SameLine();
            if (ImGui::Button("Remove Component"))
            {
                RemoveComponent(selectedComponentIndex);
            }
        }

        ImGui::Spacing();

        if (App->GetEditorUIModule()->RenderTransformWidget(localTransform, globalTransform, parentTransform))
        {
            UpdateTransformForGOBranch();
        }

        // Casting to use ImGui to set values and at the same type keep the enum type for the variable
        ImGui::SeparatorText("Mobility");
        ImGui::RadioButton("Static", &mobilitySettings, STATIC);
        ImGui::SameLine();
        ImGui::RadioButton("Dynamic", &mobilitySettings, DYNAMIC);

        ImGui::SeparatorText("Component hierarchy");

        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
        ImGui::BeginChild(
            "ComponentHierarchyWrapper", ImVec2(0, 200), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeY
        );
        ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                                        ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf;

        for (const auto& component : components)
        {
            ImGuiTreeNodeFlags node_flag = base_flags;
            if (selectedComponentIndex == component.first)
            {
                node_flag |= ImGuiTreeNodeFlags_Selected;
            }
            if (ImGui::TreeNodeEx((void*)component.second->GetUID(), node_flag, component.second->GetName()))
            {
                if (ImGui::IsItemClicked())
                {
                    selectedComponentIndex == component.first ? selectedComponentIndex = COMPONENT_NONE
                                                              : selectedComponentIndex = component.first;
                }
                ImGui::TreePop();
            }
        }

        ImGui::EndChild();
        ImGui::PopStyleVar();

        ImGui::Spacing();

        ImGui::SeparatorText("Component configuration");

        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
        ImGui::BeginChild(
            "ComponentInspectorWrapper", ImVec2(0, 50), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeY
        );

        if (components.find(selectedComponentIndex) != components.end())
        {
            components.at(selectedComponentIndex)->RenderEditorInspector();
        }

        ImGui::EndChild();
        ImGui::PopStyleVar();

        ImGui::End();

        if (App->GetEditorUIModule()->RenderImGuizmo(localTransform, globalTransform, parentTransform))
        {
            UpdateTransformForGOBranch();
        }
    }
    else
    {
        ImGui::End();
    }
}

void GameObject::UpdateTransformForGOBranch() const
{
    std::stack<UID> childrenBuffer;
    childrenBuffer.push(uid);

    while (!childrenBuffer.empty())
    {
        GameObject* gameObject = App->GetSceneModule()->GetGameObjectByUUID(childrenBuffer.top());
        childrenBuffer.pop();
        if (gameObject != nullptr)
        {
            gameObject->OnTransformUpdated();
            for (UID child : gameObject->GetChildren())
                childrenBuffer.push(child);
        }
    }

    App->GetSceneModule()->RegenerateTree();
}

const MeshComponent* GameObject::GetMeshComponent() const
{
    if (components.find(COMPONENT_MESH) != components.end())
    {
        return dynamic_cast<const MeshComponent*>(components.at(COMPONENT_MESH));
    }
    return nullptr;
}

void GameObject::AddModel(UID meshUid, UID materialUid) const
{
    if (components.find(COMPONENT_MESH) != components.end())
    {
        MeshComponent* mesh = static_cast<MeshComponent*>(components.at(COMPONENT_MESH));
        mesh->AddMesh(meshUid);
        mesh->AddMaterial(materialUid);
    }
}

void GameObject::OnTransformUpdated()
{
    globalTransform = GetParentGlobalTransform() * localTransform;
    globalOBB       = localAABB.Transform(globalTransform);
    globalAABB      = AABB(globalOBB);
}

void GameObject::UpdateLocalTransform(const float4x4& parentGlobalTransform)
{
    localTransform = parentGlobalTransform.Inverted() * globalTransform;
}

void GameObject::RenderHierarchyNode(UID& selectedGameObjectUUID)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    bool hasChildren         = !children.empty();

    if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    if (selectedGameObjectUUID == uid) flags |= ImGuiTreeNodeFlags_Selected;

    ImGui::PushID(static_cast<int>(uid));

    bool nodeOpen = false;

    if (isRenaming && currentRenamingUID == uid)
    {
        nodeOpen = ImGui::TreeNodeEx("##RenamingNode", flags, "");
        RenameGameObjectHierarchy();
    }
    else
    {
        nodeOpen = ImGui::TreeNodeEx(name.c_str(), flags);
    }

    HandleNodeClick(selectedGameObjectUUID);
    RenderContextMenu();

    if (nodeOpen && hasChildren)
    {
        for (UID childUUID : children)
        {
            GameObject* childGameObject = App->GetSceneModule()->GetGameObjectByUUID(childUUID);
            if (childGameObject && childUUID != uid)
            {
                childGameObject->RenderHierarchyNode(selectedGameObjectUUID);
            }
        }

        ImGui::TreePop();
    }

    ImGui::PopID();
}

void GameObject::HandleNodeClick(UID& selectedGameObjectUUID)
{
    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        selectedGameObjectUUID = uid;
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        selectedGameObjectUUID = uid;
        ImGui::OpenPopup(("##GameObjectContextMenu" + std::to_string(uid)).c_str());
    }

    // Drag and Drop

    if (uid != App->GetSceneModule()->GetGameObjectRootUID() && ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("DRAG_DROP_GAMEOBJECT", &uid, sizeof(UID));
        ImGui::Text("Dragging %s", name.c_str());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAG_DROP_GAMEOBJECT"))
        {
            UID draggedUUID = *static_cast<const UID*>(payload->Data);
            if (draggedUUID != uid)
            {
                UpdateGameObjectHierarchy(draggedUUID);
            }
        }

        ImGui::EndDragDropTarget();
    }
}

void GameObject::RenderContextMenu()
{
    if (ImGui::BeginPopup(("##GameObjectContextMenu" + std::to_string(uid)).c_str()))
    {
        if (ImGui::MenuItem("New GameObject"))
        {
            auto newGameObject = new GameObject(uid, "new Game Object");
            App->GetSceneModule()->AddGameObject(newGameObject->GetUID(), newGameObject);
            App->GetSceneModule()->RegenerateTree();
        }

        if (ImGui::MenuItem("Rename"))
        {
            if (currentRenamingUID != INVALID_UUID && currentRenamingUID != uid)
            {
                GameObject* oldGameObject = App->GetSceneModule()->GetGameObjectByUUID(currentRenamingUID);

                if (oldGameObject)
                {
                    oldGameObject->name       = oldGameObject->renameBuffer;
                    oldGameObject->isRenaming = false;
                }
            }

            isRenaming = true;
            strncpy_s(renameBuffer, sizeof(renameBuffer), name.c_str(), _TRUNCATE);

            currentRenamingUID = uid;
        }

        if (uid != App->GetSceneModule()->GetGameObjectRootUID() && ImGui::MenuItem("Delete"))
        {
            App->GetSceneModule()->RemoveGameObjectHierarchy(uid);
            App->GetSceneModule()->RegenerateTree();
        }

        if (ImGui::Checkbox("Draw nodes", &drawNodes)) OnDrawConnectionsToggle();

        ImGui::EndPopup();
    }
}

void GameObject::RenameGameObjectHierarchy()
{
    ImGui::SameLine();

    if (ImGui::InputText(
            "##RenameInput", renameBuffer, IM_ARRAYSIZE(renameBuffer), ImGuiInputTextFlags_EnterReturnsTrue
        ))
    {
        name               = renameBuffer;
        isRenaming         = false;
        currentRenamingUID = INVALID_UUID;
    }

    bool isClickedOutside =
        !ImGui::IsItemFocused() &&
        (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) &&
        !ImGui::IsAnyItemHovered();

    if (isClickedOutside)
    {
        name               = renameBuffer;
        isRenaming         = false;
        currentRenamingUID = INVALID_UUID;
    }
}

void GameObject::UpdateGameObjectHierarchy(UID sourceUID)
{
    GameObject* sourceGameObject = App->GetSceneModule()->GetGameObjectByUUID(sourceUID);

    if (sourceGameObject != nullptr)
    {
        UID oldParentUUID = sourceGameObject->GetParent();
        sourceGameObject->SetParent(uid);

        GameObject* oldParentGameObject = App->GetSceneModule()->GetGameObjectByUUID(oldParentUUID);

        if (oldParentGameObject)
        {
            oldParentGameObject->RemoveGameObject(sourceGameObject->GetUID());
        }

        AddGameObject(sourceGameObject->GetUID());

        sourceGameObject->UpdateLocalTransform(globalTransform);
        sourceGameObject->UpdateTransformForGOBranch();
    }
}

void GameObject::OnAABBUpdated()
{
    localAABB = AABB(DEFAULT_GAME_OBJECT_AABB);

    for (auto& component : components)
    {
        localAABB.Enclose(component.second->GetLocalAABB());
    }
    OnTransformUpdated();
}

void GameObject::Render() const
{
    for (auto& component : components)
    {
        component.second->Render();
    }
}

void GameObject::RenderEditor()
{
    if (App->GetEditorUIModule()->inspectorMenu)
    {
        RenderEditorInspector();
    }
    if (App->GetEditorUIModule()->hierarchyMenu)
    {
        App->GetSceneModule()->RenderHierarchyUI(App->GetEditorUIModule()->hierarchyMenu);
    }
}

void GameObject::DrawGizmos() const
{
    if (drawNodes) DrawNodes();
}

const float4x4& GameObject::GetParentGlobalTransform() const
{
    GameObject* parent = App->GetSceneModule()->GetGameObjectByUUID(parentUID);
    if (parent != nullptr)
    {
        return parent->GetGlobalTransform();
    }
    return float4x4::identity;
}

void GameObject::DrawNodes() const
{
    DebugDrawModule* debug = App->GetDebugDrawModule();

    debug->DrawLine(
        GetGlobalTransform().TranslatePart(),
        GetParentGlobalTransform().TranslatePart() - GetGlobalTransform().TranslatePart(),
        GetGlobalTransform().TranslatePart().Distance(GetParentGlobalTransform().TranslatePart()), float3(1, 1, 1),
        false
    );

    debug->DrawAxisTriad(GetGlobalTransform(), false);
}

void GameObject::OnDrawConnectionsToggle()
{
    for (const UID childUID : children)
    {
        GameObject* childObject = App->GetSceneModule()->GetGameObjectByUUID(childUID);
        childObject->drawNodes  = drawNodes;
        childObject->OnDrawConnectionsToggle();
    }
}

bool GameObject::CreateComponent(const ComponentType componentType)
{
    if (components.find(componentType) == components.end())
    // TODO Allow override of components after displaying an info box
    {
        Component* createdComponent = ComponentUtils::CreateEmptyComponent(componentType, LCG().IntFast(), uid);
        if (createdComponent != nullptr)
        {
            components.insert({componentType, createdComponent});
            selectedComponentIndex = componentType;
            return true;
        }
    }

    return false;
}

bool GameObject::RemoveComponent(ComponentType componentType)
{
    if (components.find(componentType) != components.end())
    {
        delete components.at(componentType);
        components.erase(componentType);
        selectedComponentIndex = COMPONENT_NONE;
    }
    return false;
}