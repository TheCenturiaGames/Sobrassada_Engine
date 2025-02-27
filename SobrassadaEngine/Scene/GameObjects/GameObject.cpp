#include "GameObject.h"

#include "Application.h"
#include "EditorUIModule.h"
#include "Root/RootComponent.h"
#include "SceneModule.h"

#include "imgui.h"

GameObject::GameObject(std::string name) : name(name)
{
    uid       = GenerateUID();
    parentUID = INVALID_UID;
    globalAABB.SetNegativeInfinity();
}

GameObject::GameObject(UID parentUID, std::string name) : parentUID(parentUID), name(name)
{
    uid = GenerateUID();
    globalAABB.SetNegativeInfinity();
    CreateRootComponent();
}

GameObject::GameObject(UID parentUID, std::string name, UID rootComponentUID) : parentUID(parentUID), name(name)
{
    rootComponent = dynamic_cast<RootComponent*>(App->GetSceneModule()->GetComponentByUID(rootComponentUID));
}

GameObject::GameObject(const rapidjson::Value& initialState) : uid(initialState["UID"].GetUint64())
{
    parentUID = initialState["ParentUID"].GetUint64();
    name       = initialState["Name"].GetString();

    if (initialState.HasMember("Children") && initialState["Children"].IsArray())
    {
        const rapidjson::Value& initChildren = initialState["Children"];

        for (rapidjson::SizeType i = 0; i < initChildren.Size(); i++)
        {
            children.push_back(initChildren[i].GetUint64());
        }
    }
    rootComponent = dynamic_cast<RootComponent*>(
        App->GetSceneModule()->GetComponentByUID(initialState["RootComponentUID"].GetUint64())
    );
}

GameObject::~GameObject()
{
    App->GetSceneModule()->RemoveComponent(rootComponent->GetUID());
    delete rootComponent;
    rootComponent = nullptr;
}

bool GameObject::CreateRootComponent()
{

    rootComponent = dynamic_cast<RootComponent*>(
        ComponentUtils::CreateEmptyComponent(COMPONENT_ROOT, LCG().IntFast(), uid, -1, Transform())
    ); // TODO Add the gameObject UUID as parent?

    // TODO Replace parentUUID above with the UUID of this gameObject
    App->GetSceneModule()->AddComponent(rootComponent->GetUID(), rootComponent);
    return true;
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

void GameObject::OnEditor()
{
}

void GameObject::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    targetState.AddMember("UID", uid, allocator);
    targetState.AddMember("ParentUID", parentUID, allocator);
    targetState.AddMember("Name", rapidjson::Value(name.c_str(), allocator), allocator);

    rapidjson::Value valChildren(rapidjson::kArrayType);

    for (const UID child : children)
    {
        valChildren.PushBack(child, allocator);
    }

    targetState.AddMember("Children", valChildren, allocator);
    targetState.AddMember("RootComponentUID", rootComponent->GetUID(), allocator);
}

void GameObject::SaveToLibrary()
{
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
        for (UID childUID : children)
        {
            GameObject* childGameObject = App->GetSceneModule()->GetGameObjectByUID(childUID);
            if (childGameObject && childUID != uid)
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
            UID draggedUUID = *reinterpret_cast<const UID*>(payload->Data);
            if (draggedUUID != uid)
            {
                if (UpdateGameObjectHierarchy(draggedUUID, uid))
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
    if (ImGui::BeginPopup(("##GameObjectContextMenu" + std::to_string(uid)).c_str()))
    {
        if (ImGui::MenuItem("New GameObject"))
        {
            GameObject* newGameObject = new GameObject(uid, "new Game Object");
            App->GetSceneModule()->GetGameObjectByUID(uid)->AddGameObject(newGameObject->GetUID());
            App->GetSceneModule()->AddGameObject(newGameObject->GetUID(), newGameObject);
            ComponentGlobalTransformUpdated();
        }

        if (ImGui::MenuItem("Rename"))
        {
            if (currentRenamingUID != INVALID_UID && currentRenamingUID != uid)
            {
                GameObject* oldGameObject = App->GetSceneModule()->GetGameObjectByUID(currentRenamingUID);

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
        currentRenamingUID = INVALID_UID;
    }

    bool isClickedOutside =
        !ImGui::IsItemFocused() &&
        (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) &&
        !ImGui::IsAnyItemHovered();

    if (isClickedOutside)
    {
        name               = renameBuffer;
        isRenaming         = false;
        currentRenamingUID = INVALID_UID;
    }
}

bool GameObject::UpdateGameObjectHierarchy(UID sourceUID, UID targetUID)
{
    GameObject* sourceGameObject = App->GetSceneModule()->GetGameObjectByUID(sourceUID);
    GameObject* targetGameObject = App->GetSceneModule()->GetGameObjectByUID(targetUID);

    if (!sourceGameObject || !targetGameObject) return false;

    UID oldParentUID = sourceGameObject->GetParent();
    sourceGameObject->SetParent(targetUID);

    GameObject* oldParentGameObject = App->GetSceneModule()->GetGameObjectByUID(oldParentUID);

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
            rootComponent->RenderGuizmo();
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
        GameObject* gameObject = App->GetSceneModule()->GetGameObjectByUID(child);

        if (gameObject != nullptr)
        {
            globalAABB.Enclose(gameObject->GetAABB());
        }
    }

    if (parentUID != INVALID_UID) // Filters the case of Scene GameObject (which parent is INVALID_UUID)
    {
        GameObject* parentGameObject = App->GetSceneModule()->GetGameObjectByUID(parentUID);

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
        GameObject* childGameObject = App->GetSceneModule()->GetGameObjectByUID(child);

        if (childGameObject != nullptr)
        {
            globalAABB.Enclose(childGameObject->rootComponent->TransformUpdated(
                rootComponent == nullptr ? Transform::identity : rootComponent->GetGlobalTransform()
            ));
        }
    }
}

const Transform& GameObject::GetGlobalTransform() const
{
    return rootComponent->GetGlobalTransform();
}

const Transform& GameObject::GetParentGlobalTransform()
{
    GameObject* parent = App->GetSceneModule()->GetGameObjectByUID(parentUID);
    if (parent != nullptr)
    {
        return parent->GetGlobalTransform();
    }
    return Transform::identity;
}