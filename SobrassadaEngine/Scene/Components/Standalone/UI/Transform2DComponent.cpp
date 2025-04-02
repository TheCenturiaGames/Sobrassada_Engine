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
    valAnchors.PushBack(anchorsY.x, allocator);
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

        ImGui::PushItemWidth(75);

        // Position X / Left
        if (anchorsX.x == anchorsX.y)
        {
            if (ImGui::DragFloat("Pos X", &position.x, 0.1f)) UpdateParent3DTransform();
        }
        else
        {
            float leftMargin = margins.x;
            if (ImGui::DragFloat("Left", &margins.x, 0.1f)) OnLeftMarginChanged();
        }

        ImGui::SameLine();

        // Position Y / Top
        if (anchorsY.x == anchorsY.y)
        {
            if (ImGui::DragFloat("Pos Y", &position.y, 0.1f)) UpdateParent3DTransform();
        }
        else
        {
            if (ImGui::DragFloat("Top", &margins.z, 0.1f)) OnTopMarginChanged();
        }

        // Width / Right
        if (anchorsX.x == anchorsX.y)
        {
            if (ImGui::DragFloat("Width", &size.x, 0.1f)) OnSizeChanged();
        }
        else
        {
            if (ImGui::DragFloat("Right", &margins.y, 0.1f)) OnRightMarginChanged();
        }

        ImGui::SameLine();

        // Height / Bottom
        if (anchorsY.x == anchorsY.y)
        {
            if (ImGui::DragFloat("Height", &size.y, 0.1f)) OnSizeChanged();
        }
        else
        {
            if (ImGui::DragFloat("Bottom", &margins.w, 0.1f)) OnBottomMarginChanged();
        }

        ImGui::PopItemWidth();

        ImGui::DragFloat2("Pivot", &pivot[0], 0.01f, 0.0f, 1.0f);
        ImGui::Separator();
        ImGui::Text("Anchors");

        if (ImGui::DragFloat2("X-axis bounds", &anchorsX.x, 0.001f, 0.0f, 1.0f)) OnAnchorsUpdated();
        if (ImGui::DragFloat2("Y-axis bounds", &anchorsY.x, 0.001f, 0.0f, 1.0f)) OnAnchorsUpdated();

        ImGui::InputFloat2("Debug pos", &position.x);
        ImGui::InputFloat2("Debug size", &size.x);
    }
}

void Transform2DComponent::UpdateParent3DTransform()
{
    // Get the parent local transform and update it according to the Transform2D. The parentTransform global
    // position is subtracted in order to get the anchor in a local position

    float2 localPos;
    if (anchorsX.x == anchorsX.y && !IsRootTransform2D())
        localPos.x = GetAnchorXPos(anchorsX.x) + position.x - parentTransform->GetGlobalPosition().x;
    else localPos.x = position.x;

    if (anchorsY.x == anchorsY.y && !IsRootTransform2D())
        localPos.y = GetAnchorYPos(anchorsY.x) + position.y - parentTransform->GetGlobalPosition().y;
    else localPos.y = position.y;

    float4x4 transform = parent->GetLocalTransform();
    transform.SetTranslatePart(localPos.x, localPos.y, 0);
    parent->SetLocalTransform(transform);
}

void Transform2DComponent::UpdateChildren2DTransforms()
{
}

void Transform2DComponent::OnTransform3DUpdated(const float4x4& globalTransform3D)
{
    if (marginUpdated)
    {
        marginUpdated = false;
        return;
    }

    if (anchorsX.x == anchorsX.y)
    {
        position.x = globalTransform3D.TranslatePart().x - GetAnchorXPos(anchorsX.x);
    }
    else
    {
        position.x = parent->GetLocalTransform().TranslatePart().x;
        UpdateHorizontalMargins();
    }

    if (anchorsY.x == anchorsY.y)
    {
        position.y = globalTransform3D.TranslatePart().y - GetAnchorYPos(anchorsY.x);
    }
    else
    {
        position.y = parent->GetLocalTransform().TranslatePart().y;
        UpdateVerticalMargins();
    }

    // Like in Unity, when anchors are separated probably position will be calculated in a different way
}

float2 Transform2DComponent::GetRenderingPosition() const
{
    // Gets the position to use for rendering the widget (at the top-left corner of the widget space)
    return float2(GetGlobalPosition().x - (size.x * pivot.x), GetGlobalPosition().y + (size.y * (1 - pivot.y)));
}

float2 Transform2DComponent::GetGlobalPosition() const
{
    if (anchorsX.x == anchorsX.y)
        return float2(GetAnchorXPos(anchorsX.x) + position.x, GetAnchorYPos(anchorsY.x) + position.y);
    else return float2(parent->GetGlobalTransform().TranslatePart().x, parent->GetGlobalTransform().TranslatePart().y);
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
    if (anchorsX.x == anchorsX.y)
        position.x = parent->GetGlobalTransform().TranslatePart().x - GetAnchorXPos(anchorsX.x);
    else UpdateHorizontalMargins();

    if (anchorsY.x == anchorsY.y)
        position.y = parent->GetGlobalTransform().TranslatePart().y - GetAnchorYPos(anchorsY.x);
    else UpdateVerticalMargins();
}

void Transform2DComponent::OnSizeChanged()
{
    for (const auto& child : childTransforms)
    {
        child->OnAnchorsUpdated();
        child->UpdateParent3DTransform();
    }
}

void Transform2DComponent::OnLeftMarginChanged()
{
    marginUpdated = true;
    size.x =
        abs((GetAnchorXPos(anchorsX.x) + margins.x) -
            (parent->GetGlobalTransform().TranslatePart().x + (size.x * (1 - pivot.x))));

    position.x = ((GetAnchorXPos(anchorsX.x) + margins.x) + (GetAnchorXPos(anchorsX.y) + margins.y)) / 2;
    if (!IsRootTransform2D()) position.x -= parentTransform->GetGlobalPosition().x;

    UpdateParent3DTransform();
}

void Transform2DComponent::OnRightMarginChanged()
{
    marginUpdated = true;

    size.x =
        abs((GetAnchorXPos(anchorsX.y) + margins.y) -
            (parent->GetGlobalTransform().TranslatePart().x - (size.x * pivot.x)));

    position.x = ((GetAnchorXPos(anchorsX.x) + margins.x) + (GetAnchorXPos(anchorsX.y) + margins.y)) / 2;
    if (!IsRootTransform2D()) position.x -= parentTransform->GetGlobalPosition().x;

    UpdateParent3DTransform();
}

void Transform2DComponent::OnTopMarginChanged()
{
    marginUpdated = true;

    size.y =
        abs((GetAnchorYPos(anchorsY.y) + margins.z) -
            (parent->GetGlobalTransform().TranslatePart().y - (size.y * pivot.y)));

    position.y = ((GetAnchorYPos(anchorsY.y) + margins.z) + (GetAnchorYPos(anchorsY.x) + margins.w)) / 2;
    if (!IsRootTransform2D()) position.y -= parentTransform->GetGlobalPosition().y;

    UpdateParent3DTransform();
}

void Transform2DComponent::OnBottomMarginChanged()
{
    marginUpdated = true;

    size.y =
        abs((GetAnchorYPos(anchorsY.x) + margins.w) -
            (parent->GetGlobalTransform().TranslatePart().y + (size.y * (1 - pivot.y))));

    position.y = ((GetAnchorYPos(anchorsY.y) + margins.z) + (GetAnchorYPos(anchorsY.x) + margins.w)) / 2;
    if (!IsRootTransform2D()) position.y -= parentTransform->GetGlobalPosition().y;

    UpdateParent3DTransform();
}

void Transform2DComponent::UpdateHorizontalMargins()
{
    margins.x = (parent->GetGlobalTransform().TranslatePart().x - (size.x * pivot.x)) - GetAnchorXPos(anchorsX.x);
    margins.y = (parent->GetGlobalTransform().TranslatePart().x + (size.x * (1 - pivot.x))) - GetAnchorXPos(anchorsX.y);
}

void Transform2DComponent::UpdateVerticalMargins()
{
    margins.z = (parent->GetGlobalTransform().TranslatePart().y + (size.y * (1 - pivot.y))) - GetAnchorYPos(anchorsY.y);
    margins.w = (parent->GetGlobalTransform().TranslatePart().y - (size.y * pivot.y)) - GetAnchorYPos(anchorsY.x);
}