#include "CanvasComponent.h"

#include "Application.h"
#include "ButtonComponent.h"
#include "CameraModule.h"
#include "DebugDrawModule.h"
#include "GameObject.h"
#include "GameUIModule.h"
#include "ImageComponent.h"
#include "SceneModule.h"
#include "ShaderModule.h"
#include "Transform2DComponent.h"
#include "UILabelComponent.h"
#include "WindowModule.h"

#include "glew.h"
#include "imgui.h"
#include <queue>

CanvasComponent::CanvasComponent(UID uid, GameObject* parent) : Component(uid, parent, "Canvas", COMPONENT_CANVAS)
{
    width  = (float)App->GetWindowModule()->GetWidth();
    height = (float)App->GetWindowModule()->GetHeight();
}

CanvasComponent::CanvasComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent)
{
    width      = initialState["Width"].GetFloat();
    height     = initialState["Height"].GetFloat();
    renderMode = static_cast<CanvasRenderMode>(initialState["RenderMode"].GetInt());
}

CanvasComponent::~CanvasComponent()
{
    App->GetGameUIModule()->RemoveCanvas(this);
}

void CanvasComponent::Init()
{
    App->GetGameUIModule()->AddCanvas(this);

    // Calculate position canvas whether is WS or SO
    float originX      = IsInWorldSpace() ? parent->GetGlobalTransform().TranslatePart().x : 0.0f;
    float originY      = IsInWorldSpace() ? parent->GetGlobalTransform().TranslatePart().y : 0.0f;
    localComponentAABB = AABB(
        float3(originX - (width / 2.0f), originY - (height / 2.0f), 0),
        float3(originX + (width / 2.0f), originY + (height / 2.0f), 0)
    );
}

void CanvasComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);

    // Canvas dimensions
    targetState.AddMember("Width", width, allocator);
    targetState.AddMember("Height", height, allocator);

    // Render mode: 0 = Overlay, 1 = WorldSpace
    targetState.AddMember("RenderMode", static_cast<int>(renderMode), allocator);
}

void CanvasComponent::Clone(const Component* other)
{
    if (other->GetType() == ComponentType::COMPONENT_CANVAS)
    {
        const CanvasComponent* otherCanvas = static_cast<const CanvasComponent*>(other);
        width                              = otherCanvas->width;
        height                             = otherCanvas->height;
        renderMode                         = otherCanvas->renderMode;
    }
    else
    {
        GLOG("It is not possible to clone a component of a different type!");
    }
}

void CanvasComponent::Update(float deltaTime)
{
    if (!IsEffectivelyEnabled()) return;
    if (renderMode != CanvasRenderMode::ScreenSpaceOverlay) return;

    // Update canvas if window size changed (only in overlay space)
    const auto& size = App->GetSceneModule()->GetScene()->GetWindowSize();
    float newWidth   = std::get<0>(size);
    float newHeight  = std::get<1>(size);
    if (width != newWidth || height != newHeight)
    {
        OnWindowResize(newWidth, newHeight);
    }
}

//Draw the canvas area in WorldSpace
void CanvasComponent::RenderDebug(float deltaTime)
{
    float3 defaultColor = float3(1, 1, 1);
    RenderDebug(deltaTime, defaultColor);
}

void CanvasComponent::RenderDebug(float deltaTime, const float3& color)
{
    if (!IsEffectivelyEnabled()) return;

    float x = parent->GetGlobalTransform().TranslatePart().x;
    float y = parent->GetGlobalTransform().TranslatePart().y;

    App->GetDebugDrawModule()->DrawLine(float3(x - width / 2, y + height / 2, 0), float3::unitX, width, color);
    App->GetDebugDrawModule()->DrawLine(float3(x - width / 2, y - height / 2, 0), float3::unitX, width, color);
    App->GetDebugDrawModule()->DrawLine(float3(x - width / 2, y + height / 2, 0), -float3::unitY, height, color);
    App->GetDebugDrawModule()->DrawLine(float3(x + width / 2, y + height / 2, 0), -float3::unitY, height, color);
} 


void CanvasComponent::RenderUI()
{
    if (!IsEffectivelyEnabled()) return;

    const int uiProgram = App->GetShaderModule()->GetUIWidgetProgram();
    if (uiProgram == -1)
    {
        GLOG("Error with UI Program");
        return;
    }
    glUseProgram(uiProgram);

    float4x4 view = float4x4::identity;
    float4x4 proj = float4x4::identity;

    if (renderMode == CanvasRenderMode::WorldSpace)
    {
        view = App->GetCameraModule()->GetViewMatrix();
        proj = App->GetCameraModule()->GetProjectionMatrix();
    }
    else // ScreenSpaceOverlay
    {
        proj = float4x4::D3DOrthoProjLH(-1, 1, width, height);
    }

    for (const GameObject* child : sortedChildren)
    {
        if (!child->IsGloballyEnabled()) continue;

        Transform2DComponent* transform = child->GetComponent<Transform2DComponent*>();
        if (transform) transform->RenderWidgets();

        UILabelComponent* uiLabel = child->GetComponent<UILabelComponent*>();
        if (uiLabel) uiLabel->RenderUI(view, proj);

        ImageComponent* image = child->GetComponent<ImageComponent*>();
        if (image) image->RenderUI(view, proj);
    }
}

void CanvasComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();

    if (enabled)
    {
        ImGui::Text("Canvas");

        const char* modes[] = {"Screen Space - Overlay", "World Space"};
        int currentMode     = static_cast<int>(renderMode);
        if (ImGui::Combo("Render Mode", &currentMode, modes, IM_ARRAYSIZE(modes)))
        {
            renderMode = static_cast<CanvasRenderMode>(currentMode);
        }
    }
}

void CanvasComponent::OnWindowResize(const float width, const float height)
{
    this->width        = width;
    this->height       = height;

    localComponentAABB = AABB(
        float3(
            parent->GetGlobalTransform().TranslatePart().x - (width / 4.0f),
            parent->GetGlobalTransform().TranslatePart().y - (height / 4.0f), 0
        ),
        float3(
            parent->GetGlobalTransform().TranslatePart().x + (width / 4.0f),
            parent->GetGlobalTransform().TranslatePart().y + (height / 4.0f), 0
        )
    );
}

void CanvasComponent::UpdateChildren()
{
    if (!IsEffectivelyEnabled()) return;

    // TODO: Right now this updates the children list every frame in case they are reordered in hierarchy.
    // To be more optimal, this could be called only when a gameObject is dragged around the hierarchy
    sortedChildren.clear();

    std::queue<UID> children;

    for (const UID child : parent->GetChildren())
    {
        children.push(child);
    }

    while (!children.empty())
    {
        const GameObject* currentObject = App->GetSceneModule()->GetScene()->GetGameObjectByUID(children.front());
        sortedChildren.push_back(currentObject);
        children.pop();

        for (const UID child : currentObject->GetChildren())
        {
            children.push(child);
        }
    }
}

void CanvasComponent::UpdateMousePosition(const float2& mousePos)
{
    if (!IsEffectivelyEnabled()) return;

    // Only interact with elements if canvas is in screen mode
    if (renderMode == CanvasRenderMode::WorldSpace) return;

    hoveredButton    = nullptr;
    bool buttonFound = false;

    for (int i = (int)sortedChildren.size() - 1; i >= 0; --i)
    {
        // Update all buttons
        ButtonComponent* currentButton = sortedChildren[i]->GetComponent<ButtonComponent*>();
        if (currentButton && currentButton->UpdateMousePosition(mousePos, buttonFound))
        {
            hoveredButton = currentButton;
            buttonFound   = true;
        }
    }
}

void CanvasComponent::OnMouseButtonPressed() const
{
    if (hoveredButton) hoveredButton->OnClick();
}

void CanvasComponent::OnMouseButtonReleased() const
{
    if (hoveredButton) hoveredButton->OnRelease();
}

bool CanvasComponent::IsInWorldSpace() const
{
    return renderMode == CanvasRenderMode::WorldSpace;
}

float CanvasComponent::GetScreenScale() const
{
    float scaleX = width / referenceWidth;
    float scaleY = height / referenceHeight;
    return std::min(scaleX, scaleY);
}
