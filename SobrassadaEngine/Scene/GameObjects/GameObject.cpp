#include "GameObject.h"

#include "Application.h"
#include "Component.h"
#include "DebugDrawModule.h"
#include "EditorUIModule.h"
#include "PrefabManager.h"
#include "SceneModule.h"
#include "Standalone/AnimationComponent.h"
#include "Standalone/MeshComponent.h"
#include "Standalone/UI/Transform2DComponent.h"

#include "imgui.h"
#include <set>
#include <stack>

GameObject::GameObject(const std::string& name) : name(name)
{
    uid       = GenerateUID();
    parentUID = INVALID_UID;

    localAABB = AABB();
    localAABB.SetNegativeInfinity();

    globalOBB  = OBB(localAABB);
    globalAABB = AABB(globalOBB);
}

GameObject::GameObject(UID parentUID, const std::string& name) : parentUID(parentUID), name(name)
{
    uid       = GenerateUID();

    localAABB = AABB();
    localAABB.SetNegativeInfinity();

    globalOBB  = OBB(localAABB);
    globalAABB = AABB(globalOBB);
}

GameObject::GameObject(UID parentUID, const std::string& name, UID uid) : parentUID(parentUID), name(name), uid(uid)
{
    localAABB = AABB();
    localAABB.SetNegativeInfinity();

    globalOBB  = OBB(localAABB);
    globalAABB = AABB(globalOBB);
}

GameObject::GameObject(UID parentUID, GameObject* refObject)
    : parentUID(parentUID), name(refObject->name), localTransform(refObject->localTransform),
      globalTransform(refObject->globalTransform)
{
    uid       = GenerateUID();

    localAABB = AABB();
    localAABB.SetNegativeInfinity();

    globalOBB        = OBB(localAABB);
    globalAABB       = AABB(globalOBB);
    isTopParent      = refObject->isTopParent;
    mobilitySettings = refObject->mobilitySettings;

    position         = refObject->position;
    rotation         = refObject->rotation;
    scale            = refObject->scale;
    prefabUID        = refObject->prefabUID;

    // Must make a copy of each manually
    for (const auto& component : refObject->components)
    {
        CreateComponent(component.first);
        Component* newComponent = GetComponentByType(component.first);
        newComponent->Clone(component.second);
    }

    OnAABBUpdated();
}

GameObject::GameObject(const rapidjson::Value& initialState) : uid(initialState["UID"].GetUint64())
{
    parentUID              = initialState["ParentUID"].GetUint64();
    name                   = initialState["Name"].GetString();
    selectedComponentIndex = COMPONENT_NONE;
    mobilitySettings       = initialState["Mobility"].GetInt();

    if (initialState.HasMember("Enabled")) enabled = initialState["Enabled"].GetBool();

    if (initialState.HasMember("IsTopParent")) isTopParent = initialState["IsTopParent"].GetBool();

    if (initialState.HasMember("PrefabUID")) prefabUID = initialState["PrefabUID"].GetUint64();

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

        position = localTransform.TranslatePart();
        rotation = localTransform.RotatePart().ToEulerXYZ();
        scale    = localTransform.GetScale();
    }

    // Deserialize Components
    if (initialState.HasMember("Components") && initialState["Components"].IsArray())
    {
        const rapidjson::Value& jsonComponents = initialState["Components"];

        for (rapidjson::SizeType i = 0; i < jsonComponents.Size(); i++)
        {
            const rapidjson::Value& jsonComponent = jsonComponents[i];

            Component* newComponent               = ComponentUtils::CreateExistingComponent(jsonComponent, this);

            if (newComponent != nullptr)
            {
                components.insert({newComponent->GetType(), newComponent});
            }
        }
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
    for (auto& component : components)
    {
        delete component.second;
    }
    components.clear();
}

void GameObject::Init()
{
    globalTransform = GetParentGlobalTransform() * localTransform;

    for (auto& component : components)
    {
        component.second->Init();
    }
}

bool GameObject::AddGameObject(UID gameObjectUID)
{
    if (std::find(children.begin(), children.end(), gameObjectUID) == children.end())
    {
        children.push_back(gameObjectUID);
        return true;
    }
    return false;
}

bool GameObject::RemoveGameObject(UID gameObjectUID)
{
    if (const auto it = std::find(children.begin(), children.end(), gameObjectUID); it != children.end())
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
    targetState.AddMember("IsTopParent", isTopParent, allocator);
    targetState.AddMember("Enabled", enabled, allocator);

    if (prefabUID != INVALID_UID) targetState.AddMember("PrefabUID", prefabUID, allocator);

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

    ImGui::SameLine();
    ImGui::Checkbox("Enabled", &enabled);

    if (uid != App->GetSceneModule()->GetScene()->GetGameObjectRootUID())
    {
        ImGui::SameLine();
        if (ImGui::Checkbox("Draw nodes", &drawNodes)) OnDrawConnectionsToggle();
        ImGui::SameLine();
        ImGui::Checkbox("Is top parent", &isTopParent);
        if (ImGui::Button("Add Component"))
        {
            ImGui::OpenPopup("ComponentSelection");
        }

        auto selectedType = App->GetEditorUIModule()->RenderResourceSelectDialog<ComponentType>(
            "ComponentSelection", App->GetEditorUIModule()->GetStandaloneComponents(), COMPONENT_NONE
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

        if (App->GetEditorUIModule()->RenderTransformWidget(
                localTransform, globalTransform, parentTransform, position, rotation, scale
            ))
        {
            UpdateTransformForGOBranch();
        }

        // Casting to use ImGui to set values and at the same type keep the enum type for the variable
        int previousMobility = mobilitySettings;
        ImGui::SeparatorText("Mobility");
        if (ImGui::RadioButton("Static", &mobilitySettings, STATIC))
        {
            if (previousMobility != mobilitySettings)
            {
                App->GetSceneModule()->GetScene()->SetStaticModified();
                App->GetSceneModule()->GetScene()->SetDynamicModified();
                UpdateMobilityHierarchy(STATIC);
            }
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Dynamic", &mobilitySettings, DYNAMIC))
        {
            if (previousMobility != mobilitySettings)
            {
                App->GetSceneModule()->GetScene()->SetStaticModified();
                App->GetSceneModule()->GetScene()->SetDynamicModified();
                UpdateMobilityHierarchy(DYNAMIC);
            }
        }

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

        if (!App->GetSceneModule()->GetInPlayMode() && App->GetSceneModule()->GetScene()->GetSceneVisible())
        {
            if (App->GetEditorUIModule()->RenderImGuizmo(
                    localTransform, globalTransform, parentTransform, position, rotation, scale
                ))
            {
                UpdateTransformForGOBranch();
            }
        }
    }
    else
    {
        ImGui::End();
    }
}

void GameObject::UpdateTransformForGOBranch()
{
    if (!IsGloballyEnabled()) return;
    App->GetSceneModule()->AddGameObjectToUpdate(this);
    std::stack<UID> childrenBuffer;
    childrenBuffer.push(uid);

    while (!childrenBuffer.empty())
    {
        GameObject* gameObject = App->GetSceneModule()->GetScene()->GetGameObjectByUID(childrenBuffer.top());
        childrenBuffer.pop();
        if (gameObject != nullptr)
        {
            App->GetSceneModule()->AddGameObjectToUpdate(gameObject);
            gameObject->OnAABBUpdated();
            for (UID child : gameObject->GetChildren())
                childrenBuffer.push(child);
        }
    }
}

Component* GameObject::GetComponentByType(ComponentType type) const
{
    if (components.find(type) != components.end())
    {
        return components.at(type);
    }
    return nullptr;
}

MeshComponent* GameObject::GetMeshComponent() const
{
    if (components.find(COMPONENT_MESH) != components.end())
    {
        return dynamic_cast<MeshComponent*>(components.at(COMPONENT_MESH));
    }
    return nullptr;
}

AnimationComponent* GameObject::GetAnimationComponent() const
{
    if (components.find(COMPONENT_ANIMATION) != components.end())
    {
        return dynamic_cast<AnimationComponent*>(components.at(COMPONENT_ANIMATION));
    }
    return nullptr;
}

void GameObject::OnTransformUpdated()
{
    globalTransform              = GetParentGlobalTransform() * localTransform;
    globalOBB                    = globalTransform * OBB(localAABB);
    globalAABB                   = AABB(globalOBB);

    MeshComponent* meshComponent = GetMeshComponent();
    if (meshComponent != nullptr)
    {
        meshComponent->OnTransformUpdated();
    }

    // If the gameObject has a transform2D, update it
    if (components.find(COMPONENT_TRANSFORM_2D) != components.end())
    {
        Transform2DComponent* transform2D = static_cast<Transform2DComponent*>(components.at(COMPONENT_TRANSFORM_2D));
        transform2D->OnTransform3DUpdated(globalTransform);
    }

    if (mobilitySettings == STATIC) App->GetSceneModule()->GetScene()->SetStaticModified();
    else App->GetSceneModule()->GetScene()->SetDynamicModified();
}

void GameObject::UpdateComponents()
{
    if (!IsGloballyEnabled()) return;
    for (const auto& component : components)
    {
        if (component.second) component.second->ParentUpdated();
    }
}

AABB GameObject::GetHierarchyAABB()
{
    AABB returnAABB;
    returnAABB.SetNegativeInfinity();
    returnAABB.Enclose(globalAABB);

    std::set<UID> visitedGameObjects;
    std::stack<UID> toVisitGameObjects;
    UID sceneRootUID = App->GetSceneModule()->GetScene()->GetGameObjectRootUID();

    visitedGameObjects.insert(uid);

    // FIRST UPDATE DOWN THE HERIARCHY
    for (UID gameObjectID : children)
        toVisitGameObjects.push(gameObjectID);

    while (!toVisitGameObjects.empty())
    {
        const UID currentUID = toVisitGameObjects.top();
        toVisitGameObjects.pop();

        if (visitedGameObjects.find(currentUID) == visitedGameObjects.end())
        {
            visitedGameObjects.insert(currentUID);
            const GameObject* currentGameObject = App->GetSceneModule()->GetScene()->GetGameObjectByUID(currentUID);

            const AABB& currentAABB             = currentGameObject->GetGlobalAABB();
            if (currentAABB.IsFinite() && !currentAABB.IsDegenerate())
                returnAABB.Enclose(currentGameObject->GetGlobalAABB());

            for (UID childID : currentGameObject->GetChildren())
                toVisitGameObjects.push(childID);
        }
    }

    return returnAABB;
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
        std::string objectName = name;
        if (prefabUID != INVALID_UID)
        {
            objectName += "(prefab " + std::to_string(prefabUID) + ')';
        }

        if (!IsGloballyEnabled())
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        }

        nodeOpen = ImGui::TreeNodeEx(objectName.c_str(), flags);

        if (!IsGloballyEnabled())
        {
            ImGui::PopStyleColor();
        }
    }

    HandleNodeClick(selectedGameObjectUUID);
    RenderContextMenu();

    if (nodeOpen && hasChildren)
    {
        for (UID childUID : children)
        {
            GameObject* childGameObject = App->GetSceneModule()->GetScene()->GetGameObjectByUID(childUID);
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

    if (uid != App->GetSceneModule()->GetScene()->GetGameObjectRootUID() && ImGui::BeginDragDropSource())
    {
        ImGui::SetDragDropPayload("DRAG_DROP_GAMEOBJECT", &uid, sizeof(UID));
        ImGui::Text("Dragging %s", name.c_str());
        ImGui::EndDragDropSource();
    }

    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DRAG_DROP_GAMEOBJECT"))
        {
            UID draggedUID        = *static_cast<const UID*>(payload->Data);
            GameObject* draggedGO = App->GetSceneModule()->GetScene()->GetGameObjectByUID(draggedUID);

            if (draggedGO != nullptr && !draggedGO->TargetIsChildren(uid))
            {
                if (draggedUID != uid)
                {
                    UpdateGameObjectHierarchy(draggedUID);
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
            auto newGameObject = new GameObject(uid, "new Game Object");
            App->GetSceneModule()->GetScene()->AddGameObject(newGameObject->GetUID(), newGameObject);

            if (newGameObject->IsStatic()) App->GetSceneModule()->GetScene()->SetStaticModified();
            else App->GetSceneModule()->GetScene()->SetDynamicModified();
        }

        if (ImGui::MenuItem("Rename"))
        {
            if (currentRenamingUID != INVALID_UID && currentRenamingUID != uid)
            {
                GameObject* oldGameObject = App->GetSceneModule()->GetScene()->GetGameObjectByUID(currentRenamingUID);

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

        const char* label = prefabUID == INVALID_UID ? "Create Prefab" : "Update Prefab";
        if (ImGui::MenuItem(label)) CreatePrefab();

        if (uid != App->GetSceneModule()->GetScene()->GetGameObjectRootUID() && ImGui::MenuItem("Delete"))
        {
            App->GetSceneModule()->GetScene()->RemoveGameObjectHierarchy(uid);
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

bool GameObject::TargetIsChildren(UID uidTarget)
{
    if (children.size() <= 0) return false;

    std::stack<UID> nodesToVisit;

    for (UID childUID : children)
    {
        nodesToVisit.push(childUID);
    }

    while (!nodesToVisit.empty())
    {
        UID currentUID = nodesToVisit.top();
        nodesToVisit.pop();

        if (currentUID == uidTarget) return true;

        GameObject* currentGO = App->GetSceneModule()->GetScene()->GetGameObjectByUID(currentUID);

        if (currentGO != nullptr)
        {
            for (UID grandChildUID : currentGO->children)
            {
                nodesToVisit.push(grandChildUID);
            }
        }
    }
    return false;
}

void GameObject::UpdateGameObjectHierarchy(UID sourceUID)
{
    GameObject* sourceGameObject = App->GetSceneModule()->GetScene()->GetGameObjectByUID(sourceUID);

    if (sourceGameObject != nullptr)
    {
        UID oldParentUID = sourceGameObject->GetParent();
        sourceGameObject->SetParent(uid);

        Component* transform2D = sourceGameObject->GetComponentByType(COMPONENT_TRANSFORM_2D);
        if (transform2D) static_cast<Transform2DComponent*>(transform2D)->OnParentChange();

        GameObject* oldParentGameObject = App->GetSceneModule()->GetScene()->GetGameObjectByUID(oldParentUID);

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
    localAABB = localAABB;
    localAABB.SetNegativeInfinity();

    for (auto& component : components)
    {
        localAABB.Enclose(component.second->GetLocalAABB());
    }
    OnTransformUpdated();
}

void GameObject::Render(float deltaTime) const
{
    for (auto& component : components)
    {
        component.second->Render(deltaTime);
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
        App->GetSceneModule()->GetScene()->RenderHierarchyUI(App->GetEditorUIModule()->hierarchyMenu);
    }
}

void GameObject::SetLocalTransform(const float4x4& newTransform)
{
    localTransform = newTransform;
    position       = localTransform.TranslatePart();
    rotation       = localTransform.RotatePart().ToEulerXYZ();
    scale          = localTransform.GetScale();
    UpdateTransformForGOBranch();
}

void GameObject::DrawGizmos() const
{
    if (!IsGloballyEnabled()) return;
    if (drawNodes) DrawNodes();
}

const float4x4& GameObject::GetParentGlobalTransform() const
{
    GameObject* parent = App->GetSceneModule()->GetScene()->GetGameObjectByUID(parentUID);
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
        GameObject* childObject = App->GetSceneModule()->GetScene()->GetGameObjectByUID(childUID);
        childObject->drawNodes  = drawNodes;
        childObject->OnDrawConnectionsToggle();
    }
}

void GameObject::UpdateMobilityHierarchy(MobilitySettings type)
{
    App->GetSceneModule()->AddGameObjectToUpdate(this);
    SetMobility(type);
    std::set<UID> visitedGameObjects;
    std::stack<UID> toVisitGameObjects;
    UID sceneRootUID = App->GetSceneModule()->GetScene()->GetGameObjectRootUID();
    // ADD "THIS" GAME OBJECT SO WHEN ASCENDING HERIARCHY WE DON'T REVISIT OUR CHILDREN
    visitedGameObjects.insert(uid);

    // FIRST UPDATE DOWN THE HERIARCHY
    for (UID gameObjectID : children)
        toVisitGameObjects.push(gameObjectID);

    while (!toVisitGameObjects.empty())
    {
        UID currentUID = toVisitGameObjects.top();
        toVisitGameObjects.pop();

        if (visitedGameObjects.find(currentUID) == visitedGameObjects.end())
        {
            visitedGameObjects.insert(currentUID);
            GameObject* currentGameObject = App->GetSceneModule()->GetScene()->GetGameObjectByUID(currentUID);

            currentGameObject->SetMobility(type);
            App->GetSceneModule()->AddGameObjectToUpdate(currentGameObject);

            for (UID childID : currentGameObject->GetChildren())
                toVisitGameObjects.push(childID);
        }
    }

    // UPDATE UP THE HERIARCHY
    if (parentUID != sceneRootUID) toVisitGameObjects.push(parentUID);

    while (!toVisitGameObjects.empty())
    {
        UID currentUID = toVisitGameObjects.top();
        toVisitGameObjects.pop();

        if (visitedGameObjects.find(currentUID) == visitedGameObjects.end())
        {
            visitedGameObjects.insert(currentUID);

            GameObject* currentGameObject = App->GetSceneModule()->GetScene()->GetGameObjectByUID(currentUID);
            if (currentGameObject)
            {
                currentGameObject->SetMobility(type);
                App->GetSceneModule()->AddGameObjectToUpdate(currentGameObject);

                for (UID childID : currentGameObject->GetChildren())
                    toVisitGameObjects.push(childID);

                if (currentGameObject->GetParent() != sceneRootUID)
                    toVisitGameObjects.push(currentGameObject->GetParent());
            }
        }
    }

    App->GetSceneModule()->GetScene()->SetStaticModified();
    App->GetSceneModule()->GetScene()->SetDynamicModified();
}

bool GameObject::CreateComponent(const ComponentType componentType)
{
    if (components.find(componentType) == components.end())
    // TODO Allow override of components after displaying an info box
    {
        Component* createdComponent = ComponentUtils::CreateEmptyComponent(componentType, GenerateUID(), this);
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

        OnAABBUpdated();
    }
    return false;
}

void GameObject::CreatePrefab()
{
    bool override = this->prefabUID != INVALID_UID;
    prefabUID     = PrefabManager::SavePrefab(this, override);

    if (override)
    {
        // Update all prefabs
        App->GetSceneModule()->GetScene()->OverridePrefabs(prefabUID);
    }
}

bool GameObject::IsGloballyEnabled() const
{
    if (!enabled) return false;
    GameObject* parent = App->GetSceneModule()->GetScene()->GetGameObjectByUID(parentUID);
    return parent ? parent->IsGloballyEnabled() : true;
}
