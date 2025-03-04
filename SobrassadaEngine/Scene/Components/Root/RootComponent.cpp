#include "RootComponent.h"

#include "Application.h"
#include "EditorUIModule.h"
#include "SceneModule.h"
#include "imgui.h"
#include "Scene/Components/Standalone/MeshComponent.h"
#include "Scene/GameObjects/GameObject.h"

#include <Algorithm/Random/LCG.h>

RootComponent::RootComponent(const UID uid, const UID uidParent, const float4x4& parentGlobalTransform)
        : Component(uid, uidParent, uid, "Root component", COMPONENT_ROOT, parentGlobalTransform)
{
    selectedUID = uid;  
}

RootComponent::RootComponent(const rapidjson::Value &initialState) : Component(initialState)
{
    selectedUID = uid;
    mobilitySettings = initialState["Mobility"].GetInt();
}


RootComponent::~RootComponent(){
    Component::~Component();
}

void RootComponent::Save(rapidjson::Value &targetState, rapidjson::Document::AllocatorType &allocator) const
{
    Component::Save(targetState, allocator);
    targetState.AddMember("Type", COMPONENT_ROOT, allocator);
    targetState.AddMember("Mobility", mobilitySettings, allocator);
}

AABB & RootComponent::TransformUpdated(const float4x4 &parentGlobalTransform)
{
    AABB& result = Component::TransformUpdated(parentGlobalTransform);

    AABBUpdatable* parentObject = GetParent();
    if (parentObject != nullptr)
    {
        parentObject->ComponentGlobalTransformUpdated();
    }

    return result;
}

void RootComponent::RenderComponentEditor()
{
    Component* selectedComponent = GetSelectedComponent();
    // TODO Replace nullptr checks with asserts (Components aquired by uid must never be nullptr, if they are, there is a bug elsewhere)
    
    if (!ImGui::Begin("Inspector", &App->GetEditorUIModule()->inspectorMenu))
    {
        ImGui::End();
        return;
    }

    //ImGui::InputText(name, test, 10, ImGuiInputTextFlags_None);
    ImGui::Text(name);
    
    if (ImGui::Button("Add Component")) // TODO Get selected component to add the new one at the correct location (By UUID)
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
            for ( const auto &componentPair : standaloneComponents ) {
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

    if (selectedUID != uid)
    {
        ImGui::SameLine();
        if (ImGui::Button("Remove Component")) 
        {
            if (selectedComponent != nullptr)
            {
                Component* selectedParentComponent = App->GetSceneModule()->GetComponentByUID(selectedComponent->GetUIDParent());
                if (selectedParentComponent != nullptr)
                {
                    selectedParentComponent->DeleteChildComponent(selectedUID);
                    selectedUID = uid;
                    selectedComponent = nullptr;
                }
            }
        }
    }
    
    RenderEditorComponentTree(selectedUID);
    
    ImGui::Spacing();

    ImGui::SeparatorText("Component configuration");

    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
    ImGui::BeginChild("ComponentInspectorWrapper", ImVec2(0, 200), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeY);
    
    if (selectedComponent != nullptr)  selectedComponent->RenderEditorInspector();
    
    ImGui::EndChild();
    ImGui::PopStyleVar();
    
    ImGui::End();
    
    if (selectedComponent != nullptr)
    {
        selectedComponent->RenderGuizmo();
    }
}

void RootComponent::RenderEditorComponentTree(const UID selectedComponentUID)
{
    ImGui::SeparatorText("Component hierarchy");
   
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
    ImGui::BeginChild("ComponentHierarchyWrapper", ImVec2(0, 200), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeY);
    ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
    if (selectedComponentUID == uid)
    {
        base_flags |= ImGuiTreeNodeFlags_Selected;
    }
    if (children.empty())
    {
        base_flags |= ImGuiTreeNodeFlags_Leaf;
    }

    const bool isExpanded = ImGui::TreeNodeEx((void*) uid, base_flags, name);
    if (ImGui::IsItemClicked() && !ImGui::IsItemToggledOpen())
        SetSelectedComponent(uid);

    HandleDragNDrop();
    
    if (isExpanded) 
    {
        for (Component* childComponent : GetChildComponents())
        {
            childComponent->RenderEditorComponentTree(selectedUID);
        }
        ImGui::TreePop();
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
}

void RootComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();
    if (enabled)
    {
        // Casting to use ImGui to set values and at the same type keep the enum type for the variable
        ImGui::SeparatorText("Mobility");
        ImGui::RadioButton("Static", &mobilitySettings, STATIC);
        ImGui::SameLine();
        ImGui::RadioButton("Dynamic", &mobilitySettings, DYNAMIC);
    }
}

void RootComponent::Update()
{
}

void RootComponent::SetSelectedComponent(const UID componentUID)
{
    selectedComponent = nullptr;
    selectedUID = componentUID;
}

bool RootComponent::CreateComponent(const ComponentType componentType)
{
    // TODO Call library to create the component with an id instead
    Component* selectedComponent = App->GetSceneModule()->GetComponentByUID(selectedUID);
    if (selectedComponent != nullptr) {
        Component* createdComponent = ComponentUtils::CreateEmptyComponent(componentType, LCG().IntFast(), selectedUID, uid, selectedComponent->GetGlobalTransform());
        if (createdComponent != nullptr)
        {
            App->GetSceneModule()->AddComponent(createdComponent->GetUID(), createdComponent);
        
            selectedComponent->AddChildComponent(createdComponent->GetUID());
            return true;
        }
    }
    return false;
}

const float4x4& RootComponent::GetParentGlobalTransform()
{
    AABBUpdatable* parentObject = GetParent();
    if (parentObject != nullptr)
    {
        return parentObject->GetParentGlobalTransform(); // parent is the gameObject, so we want to query the parent of this gameObject
    }
    return float4x4::identity;
}

Component* RootComponent::GetSelectedComponent()
{
    if (selectedComponent == nullptr)
    {
        selectedComponent = App->GetSceneModule()->GetComponentByUID(selectedUID);
        if (selectedComponent == nullptr)
        {
            // GLOG("Could not load parent with UID: %s - Object does not exist", uidParent)
        }
    }
    return selectedComponent;
}
