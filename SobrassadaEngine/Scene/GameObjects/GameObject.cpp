#include "GameObject.h"

#include "Application.h"
#include "EditorUIModule.h"
#include "Root/RootComponent.h"
#include "SceneModule.h"

#include "imgui.h"

GameObject::GameObject(std::string name) : name(name)
{
    uuid       = GenerateUID();
    parentUUID = INVALID_UUID;
    globalAABB.SetNegativeInfinity();
}

GameObject::GameObject(UID parentUUID, std::string name) : parentUUID(parentUUID), name(name)
{
    uuid = GenerateUID();
    globalAABB.SetNegativeInfinity();
}

GameObject::GameObject(UID parentUUID, std::string name, UID rootComponentUID) : parentUUID(parentUUID), name(name)
{
    
}

GameObject::GameObject(const rapidjson::Value& initialState) : uuid(initialState["UID"].GetUint64())
{
    parentUUID                  = initialState["ParentUID"].GetUint64();
    name                        = initialState["Name"].GetString();
    selectedComponentIndex      = COMPONENT_NONE;
    mobilitySettings            = initialState["Mobility"].GetInt();

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

void GameObject::LoadComponentsInGameObject(Component* component)
{
    if (!component) return;

    AddComponent(component);

    for (auto childComponentUID : component->GetChildren())
    {
        Component* childComp = App->GetSceneModule()->GetScene()->GetComponentByUID(childComponentUID);
        if (childComp != nullptr)
        {
            LoadComponentsInGameObject(childComp);
        }
    }
}

void GameObject::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    targetState.AddMember("UID", uuid, allocator);
    targetState.AddMember("ParentUID", parentUUID, allocator);
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

    if (ImGui::Button("Add Component"
        )) // TODO Get selected component to add the new one at the correct location (By UUID)
    {
        ImGui::OpenPopup("ComponentSelection");
    }

    if (ImGui::BeginPopup("ComponentSelection"))
    {
        static char searchText[255] = "";
        ImGui::InputText("Search", searchText, 255);

        ImGui::Separator();
        if (ImGui::BeginListBox("##ComponentList", ImVec2(-FLT_MIN, 5 * ImGui::GetTextLineHeightWithSpacing())))
        {
            for (const auto& componentPair : standaloneComponents)
            {
                {
                    if (componentPair.first.find(searchText) != std::string::npos)
                    {
                        if (ImGui::Selectable(componentPair.first.c_str(), false))
                        {
                            CreateComponent(componentPair.second);
                            ImGui::CloseCurrentPopup();
                        }
                    }
                }
            }
            ImGui::EndListBox();
        }
        ImGui::EndPopup();
    }

    if (selectedComponentIndex != COMPONENT_NONE)
    {
        ImGui::SameLine();
        if (ImGui::Button("Remove Component"))
        {
            RemoveComponent(selectedComponentIndex);
        }
    }

    ImGui::Spacing();

    const float4x4& parentTransform = GetParentGlobalTransform();
    if (App->GetEditorUIModule()->RenderTransformWidget(localTransform, globalTransform, parentTransform))
    {
        OnTransformUpdate(parentTransform);
    }

    ImGui::SeparatorText("Component configuration");

    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
    ImGui::BeginChild("ComponentInspectorWrapper", ImVec2(0, 200), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeY);

    if (components.find(selectedComponentIndex) != components.end())
    {
        components.at(selectedComponentIndex)->RenderEditorInspector();
    }

    ImGui::EndChild();
    ImGui::PopStyleVar();

    ImGui::End();
    
    // Casting to use ImGui to set values and at the same type keep the enum type for the variable
    ImGui::SeparatorText("Mobility");
    ImGui::RadioButton("Static", &mobilitySettings, STATIC);
    ImGui::SameLine();
    ImGui::RadioButton("Dynamic", &mobilitySettings, DYNAMIC);

    if (App->GetEditorUIModule()->RenderImGuizmo(localTransform, globalTransform, parentTransform))
    {
        OnTransformUpdate(parentTransform);
    }
}

void GameObject::RenderHierarchyNode(UID& selectedGameObjectUUID)
{
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

    bool hasChildren         = !children.empty();

    if (!hasChildren) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    if (selectedGameObjectUUID == uuid) flags |= ImGuiTreeNodeFlags_Selected;

    ImGui::PushID(static_cast<int>(uuid));

    bool nodeOpen = false;

    if (isRenaming && currentRenamingUID == uuid)
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
            if (childGameObject && childUUID != uuid)
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
        selectedGameObjectUUID = uuid;
    }

    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        selectedGameObjectUUID = uuid;
        ImGui::OpenPopup(("##GameObjectContextMenu" + std::to_string(uuid)).c_str());
    }

    // Drag and Drop

    if (uuid != App->GetSceneModule()->GetGameObjectRootUID() && ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("DRAG_DROP_GAMEOBJECT", &uuid, sizeof(UID));
        ImGui::Text("Dragging %s", name.c_str());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAG_DROP_GAMEOBJECT"))
        {
            UID draggedUUID = *reinterpret_cast<const UID*>(payload->Data);
            if (draggedUUID != uuid)
            {
                if (UpdateGameObjectHierarchy(draggedUUID, uuid))
                {
                    ComponentGlobalTransformUpdated();
                    PassAABBUpdateToParent(); // TODO: check if it works
                }
            }
        }

        ImGui::EndDragDropTarget();
    }
}

void GameObject::RenderContextMenu()
{
    if (ImGui::BeginPopup(("##GameObjectContextMenu" + std::to_string(uuid)).c_str()))
    {
        if (ImGui::MenuItem("New GameObject"))
        {
            GameObject* newGameObject = new GameObject(uuid, "new Game Object");
            App->GetSceneModule()->GetGameObjectByUUID(uuid)->AddGameObject(newGameObject->GetUID());
            App->GetSceneModule()->AddGameObject(newGameObject->GetUID(), newGameObject);
            ComponentGlobalTransformUpdated();
        }

        if (ImGui::MenuItem("Rename"))
        {
            if (currentRenamingUID != INVALID_UUID && currentRenamingUID != uuid)
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

            currentRenamingUID = uuid;
        }

        if (uuid != App->GetSceneModule()->GetGameObjectRootUID() && ImGui::MenuItem("Delete"))
        {
            App->GetSceneModule()->RemoveGameObjectHierarchy(uuid);
            // PassAABBUpdateToParent(); //TODO: check if it works
        }

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

bool GameObject::UpdateGameObjectHierarchy(UID sourceUID, UID targetUID)
{
    GameObject* sourceGameObject = App->GetSceneModule()->GetGameObjectByUUID(sourceUID);
    GameObject* targetGameObject = App->GetSceneModule()->GetGameObjectByUUID(targetUID);

    if (!sourceGameObject || !targetGameObject) return false;

    UID oldParentUUID = sourceGameObject->GetParent();
    sourceGameObject->SetParent(targetUID);

    GameObject* oldParentGameObject = App->GetSceneModule()->GetGameObjectByUUID(oldParentUUID);

    if (oldParentGameObject)
    {
        oldParentGameObject->RemoveGameObject(sourceGameObject->GetUID());
    }

    targetGameObject->AddGameObject(sourceGameObject->GetUID());

    return true;
}

void GameObject::Render()
{
    if (rootComponent != nullptr)
    {
        rootComponent->Render();
    }
}

void GameObject::RenderEditor()
{
    if (App->GetEditorUIModule()->inspectorMenu)
    {
        if (rootComponent != nullptr)
        {
            rootComponent->RenderComponentEditor();
        }
    }
    if (App->GetEditorUIModule()->hierarchyMenu)
    {
        App->GetSceneModule()->RenderHierarchyUI(App->GetEditorUIModule()->hierarchyMenu);
    }
}

void GameObject::PassAABBUpdateToParent()
{
    // TODO Update AABBs further up the gameObject tree
    globalAABB = AABB(rootComponent->GetGlobalAABB());

    for (UID child : children)
    {
        GameObject* gameObject = App->GetSceneModule()->GetGameObjectByUUID(child);

        if (gameObject != nullptr)
        {
            globalAABB.Enclose(gameObject->GetAABB());
        }
    }

    if (parentUUID != INVALID_UUID) // Filters the case of Scene GameObject (which parent is INVALID_UUID)
    {
        GameObject* parentGameObject = App->GetSceneModule()->GetGameObjectByUUID(parentUUID);

        if (parentGameObject != nullptr)
        {
            parentGameObject->PassAABBUpdateToParent();
        }
    }
    else
    {
        App->GetSceneModule()->RegenerateTree();
    }
}

void GameObject::ComponentGlobalTransformUpdated()
{
    if (rootComponent != nullptr) globalAABB = AABB(rootComponent->GetGlobalAABB());

    for (UID child : children)
    {
        GameObject* childGameObject = App->GetSceneModule()->GetGameObjectByUUID(child);

        if (childGameObject != nullptr)
        {
            globalAABB.Enclose(childGameObject->rootComponent->TransformUpdated(
                rootComponent == nullptr ? float4x4::identity : rootComponent->GetGlobalTransform()
            ));
        }
    }
}

const float4x4& GameObject::GetGlobalTransform() const
{
    return rootComponent->GetGlobalTransform();
}

const float4x4& GameObject::GetParentGlobalTransform()
{
    GameObject* parent = App->GetSceneModule()->GetGameObjectByUUID(parentUUID);
    if (parent != nullptr)
    {
        return parent->GetGlobalTransform();
    }
    return float4x4::identity;
}

bool GameObject::CreateComponent(const ComponentType componentType)
{
    if (components.find(componentType) == components.end()) // TODO Allow override of components after displaying an info box
    {
        Component* createdComponent = ComponentUtils::CreateEmptyComponent(componentType, LCG().IntFast());
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
    }
    return false;
}