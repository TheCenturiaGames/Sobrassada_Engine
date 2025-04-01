#include "Transform2DComponent.h"

#include "Application.h"
#include "CanvasComponent.h"
#include "DebugDrawModule.h"
#include "Scene.h"
#include "SceneModule.h"

#include "imgui.h"
#include <queue>

Transform2DComponent::Transform2DComponent(UID uid, GameObject* parent)
    : size(float2(400, 200)), pivot(float2(0.5f, 0.5f)), anchorsX(float2(0.5f, 0.5f)), anchorsY(float2(0.5f, 0.5f)),
      Component(uid, parent, "Transform 2D", COMPONENT_TRANSFORM_2D)
{
}

Transform2DComponent::Transform2DComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent)
{
    if (initialState.HasMember("Position") && initialState["Position"].IsArray())
    {
        const rapidjson::Value& initPosition = initialState["Position"];
        position.x                           = initPosition[0].GetFloat();
        position.y                           = initPosition[1].GetFloat();
    }

    if (initialState.HasMember("Size") && initialState["Size"].IsArray())
    {
        const rapidjson::Value& initSize = initialState["Size"];
        size.x                           = initSize[0].GetFloat();
        size.y                           = initSize[1].GetFloat();
    }

    if (initialState.HasMember("Pivot") && initialState["Pivot"].IsArray())
    {
        const rapidjson::Value& initPivot = initialState["Pivot"];
        pivot.x                           = initPivot[0].GetFloat();
        pivot.y                           = initPivot[1].GetFloat();
    }

    if (initialState.HasMember("Anchors") && initialState["Anchors"].IsArray())
    {
        const rapidjson::Value& initAnchors = initialState["Anchors"];
        anchorsX.x                          = initAnchors[0].GetFloat();
        anchorsX.y                          = initAnchors[1].GetFloat();
        anchorsY.x                          = initAnchors[2].GetFloat();
        anchorsY.y                          = initAnchors[3].GetFloat();
    }
}

Transform2DComponent::~Transform2DComponent()
{
}

void Transform2DComponent::Init()
{
    GetCanvas();

    if (!IsRootTransform2D())
    {
        parentTransform = static_cast<Transform2DComponent*>(App->GetSceneModule()
                                                                 ->GetScene()
                                                                 ->GetGameObjectByUID(parent->GetParent())
                                                                 ->GetComponentByType(COMPONENT_TRANSFORM_2D));
        parentTransform->AddChildTransform(this);
    }
}

void Transform2DComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);

    rapidjson::Value valPosition(rapidjson::kArrayType);
    valPosition.PushBack(position.x, allocator);
    valPosition.PushBack(position.y, allocator);
    targetState.AddMember("Position", valPosition, allocator);

    rapidjson::Value valSize(rapidjson::kArrayType);
    valSize.PushBack(size.x, allocator);
    valSize.PushBack(size.y, allocator);
    targetState.AddMember("Size", valSize, allocator);

    rapidjson::Value valPivot(rapidjson::kArrayType);
    valPivot.PushBack(pivot.x, allocator);
    valPivot.PushBack(pivot.y, allocator);
    targetState.AddMember("Pivot", valPivot, allocator);

    rapidjson::Value valAnchors(rapidjson::kArrayType);
    valAnchors.PushBack(anchorsX.x, allocator);
    valAnchors.PushBack(anchorsX.y, allocator);
    valAnchors.PushBack(anchorsY.y, allocator);
    valAnchors.PushBack(anchorsY.y, allocator);
    targetState.AddMember("Anchors", valAnchors, allocator);
}

void Transform2DComponent::Clone(const Component* other)
{
    if (other->GetType() == ComponentType::COMPONENT_TRANSFORM_2D)
    {
        const Transform2DComponent* otherTransform = static_cast<const Transform2DComponent*>(other);
        position                                   = otherTransform->position;
        size                                       = otherTransform->size;
        pivot                                      = otherTransform->pivot;
    }
    else
    {
        GLOG("It is not possible to clone a component of a different type!");
    }
}

void Transform2DComponent::Update(float deltaTime)
{
}

void Transform2DComponent::Render(float deltaTime)
{
    // Draw a square to show the width and height
    DebugDrawModule* debugDraw = App->GetDebugDrawModule();
    debugDraw->DrawLine(
        float3(GetRenderingPosition().x, GetRenderingPosition().y, 0), float3::unitX, size.x, float3(1, 1, 1)
    );

    debugDraw->DrawLine(
        float3(GetRenderingPosition().x + size.x, GetRenderingPosition().y, 0), -float3::unitY, size.y, float3(1, 1, 1)
    );

    debugDraw->DrawLine(
        float3(GetRenderingPosition().x, GetRenderingPosition().y - size.y, 0), float3::unitX, size.x, float3(1, 1, 1)
    );

    debugDraw->DrawLine(
        float3(GetRenderingPosition().x, GetRenderingPosition().y, 0), -float3::unitY, size.y, float3(1, 1, 1)
    );

    // Draw anchor points when selected
    if (App->GetSceneModule()->GetScene()->GetSelectedGameObject()->GetUID() == parent->GetUID())
    {
        // Top-left
        float3 x1 = float3(GetAnchorXPos(anchorsX.x), GetAnchorYPos(anchorsY.y), 0);
        debugDraw->DrawCone(x1, float3(-20, 20, 0), 10, 1);

        // Top-right
        float3 x2 = float3(GetAnchorXPos(anchorsX.y), GetAnchorYPos(anchorsY.y), 0);
        debugDraw->DrawCone(x2, float3(20, 20, 0), 10, 1);

        // Bottom-left
        float3 y1 = float3(GetAnchorXPos(anchorsX.x), GetAnchorYPos(anchorsY.x), 0);
        debugDraw->DrawCone(y1, float3(-20, -20, 0), 10, 1);

        // Bottom-right
        float3 y2 = float3(GetAnchorXPos(anchorsX.y), GetAnchorYPos(anchorsY.x), 0);
        debugDraw->DrawCone(y2, float3(20, -20, 0), 10, 1);
    }
}

void Transform2DComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();

    if (enabled)
    {
        ImGui::Text("Transform 2D");

        if (ImGui::InputFloat2("Position", &position[0]))
        {
            UpdateParent3DTransform();
        }
        ImGui::InputFloat2("Size", &size[0]);
        ImGui::DragFloat2("Pivot", &pivot[0], 0.01f, 0.0f, 1.0f);
        ImGui::Separator();
        ImGui::Text("Anchors");
        if (ImGui::DragFloat2("X-axis bounds", &anchorsX.x, 0.001f, 0.0f, 1.0f)) OnAnchorsUpdated();
        if (ImGui::DragFloat2("Y-axis bounds", &anchorsY.x, 0.001f, 0.0f, 1.0f)) OnAnchorsUpdated();
    }
}

void Transform2DComponent::UpdateParent3DTransform()
{
    float4x4 transform = parent->GetGlobalTransform();
    transform.SetTranslatePart(GetAnchorXPos(anchorsX.x) + position.x, GetAnchorYPos(anchorsY.x) + position.y, 0);
    parent->SetLocalTransform(transform);

    for (const auto& child : childTransforms)
    {
        child->OnAnchorsUpdated();
        child->UpdateParent3DTransform();
    }
}

void Transform2DComponent::UpdateChildren2DTransforms()
{

}

void Transform2DComponent::OnTransform3DUpdated(const float4x4& transform3D)
{
    position.x = transform3D.TranslatePart().x - GetAnchorXPos(anchorsX.x);
    position.y = transform3D.TranslatePart().y - GetAnchorYPos(anchorsY.x);

    for (const auto& child : childTransforms)
    {
        child->OnAnchorsUpdated();
    }

    // Like in Unity, when anchors are separated probably position will be calculated in a different way
}

float2 Transform2DComponent::GetRenderingPosition() const
{
    // Gets the position to use for rendering the widget (at the top-left corner of the widget space)
    // float2 parentPos;
    //
    // if (IsRootTransform2D())
    //    parentPos = float2(
    //        parent->GetParentGlobalTransform().TranslatePart().x, parent->GetParentGlobalTransform().TranslatePart().y
    //    );
    // else parentPos = float2(parentTransform->position.x, parentTransform->position.y);

    return float2(
        GetGlobalPosition().x - (size.x * pivot.x), GetGlobalPosition().y + (size.y * (1 - pivot.y))
    );
}

float2 Transform2DComponent::GetGlobalPosition() const
{
    return float2(GetAnchorXPos(anchorsX.x) + position.x, GetAnchorYPos(anchorsY.x) + position.y);
}

void Transform2DComponent::GetCanvas()
{
    // Search for a parent canvas iteratively
    std::queue<UID> parentQueue;
    parentQueue.push(parent->GetParent());

    Scene* scene =
        App->GetSceneModule()->GetScene(); // Save the scene here to not call for it in each iteration of the loops
    while (!parentQueue.empty())
    {
        const GameObject* currentParent = scene->GetGameObjectByUID(parentQueue.front());

        if (currentParent == nullptr) break; // If parent null it has reached the scene root

        CanvasComponent* canvas = static_cast<CanvasComponent*>(currentParent->GetComponentByType(COMPONENT_CANVAS));
        if (canvas != nullptr)
        {
            parentCanvas = canvas;
            break;
        }

        parentQueue.push(currentParent->GetParent());
        parentQueue.pop();
    }
}

bool Transform2DComponent::IsRootTransform2D() const
{
    // Returns if the parent is the canvas component, which means this gameObject is the root of the UI
    return parentCanvas->GetParentUID() == parent->GetParent();
}

float Transform2DComponent::GetAnchorXPos(const float anchor) const
{
    float anchorPos = 0;
    if (IsRootTransform2D())
        anchorPos = parent->GetParentGlobalTransform().TranslatePart().x +
                    (parentCanvas->GetWidth() * (anchor - 0.5f)); // 0.5f because canvas pivot is always in the middle
    else
        anchorPos =
            parentTransform->GetGlobalPosition().x + (parentTransform->size.x * (anchor - parentTransform->pivot.x));

    return anchorPos;
}

float Transform2DComponent::GetAnchorYPos(const float anchor) const
{
    float anchorPos = 0;
    if (IsRootTransform2D())
        anchorPos = parent->GetParentGlobalTransform().TranslatePart().y +
                    (parentCanvas->GetHeight() * (anchor - 0.5f)); // 0.5f because canvas pivot is always in the middle
    else
        anchorPos =
            parentTransform->GetGlobalPosition().y + (parentTransform->size.y * (anchor - parentTransform->pivot.y));

    return anchorPos;
}

void Transform2DComponent::OnAnchorsUpdated()
{
    // TODO: Update position according to new anchor values
    position.x = parent->GetGlobalTransform().TranslatePart().x - GetAnchorXPos(anchorsX.x);
    position.y = parent->GetGlobalTransform().TranslatePart().y - GetAnchorYPos(anchorsY.x);
}