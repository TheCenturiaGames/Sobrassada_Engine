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
#include "UIScaler.h"

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
    width                = initialState["Width"].GetFloat();
    height               = initialState["Height"].GetFloat();
    isInWorldSpace       = initialState["IsInWorldSpace"].GetBool();

    if (initialState.HasMember("ScaleMode") && initialState["ScaleMode"].IsInt())
    {
        scaleMode = static_cast<UIScaleMode>(initialState["ScaleMode"].GetInt());
    }

    if (initialState.HasMember("MatchFactor") && initialState["MatchFactor"].IsNumber())
    {
        matchFactor = initialState["MatchFactor"].GetFloat();
    }
}

CanvasComponent::~CanvasComponent()
{
    App->GetGameUIModule()->RemoveCanvas(this);
    if (uiScaler) delete uiScaler;
}

void CanvasComponent::Init()
{
    if (!IsEffectivelyEnabled()) return;

    transform2D = parent->GetComponent<Transform2DComponent*>();
    if (transform2D == nullptr)
    {
        parent->CreateComponent(COMPONENT_TRANSFORM_2D);
        transform2D = parent->GetComponent<Transform2DComponent*>();
    }

    if (!inWorldSpace && transform2D)
    {
        const int width = App->GetWindowModule()->GetWidth();
        const int height = App->GetWindowModule()->GetHeight();
        transform2D->size = float2(static_cast<float>(width), static_cast<float>(height));
        transform2D->position = float2(0.0f, 0.0f);
    }
}


void CanvasComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);

    targetState.AddMember("Width", width, allocator);
    targetState.AddMember("Height", height, allocator);
    targetState.AddMember("IsInWorldSpace", isInWorldSpace, allocator);
    targetState.AddMember("ScaleMode", static_cast<int>(scaleMode), allocator);
    targetState.AddMember("MatchFactor", matchFactor, allocator);
}

void CanvasComponent::Clone(const Component* other)
{
    if (other->GetType() == ComponentType::COMPONENT_CANVAS)
    {
        const CanvasComponent* otherCanvas = static_cast<const CanvasComponent*>(other);
        width                              = otherCanvas->width;
        height                             = otherCanvas->height;

        isInWorldSpace                     = otherCanvas->isInWorldSpace;

    }
    else
    {
        GLOG("It is not possible to clone a component of a different type!");
    }
}

void CanvasComponent::Update(float deltaTime)
{
    if (!IsEffectivelyEnabled()) return;

    const auto& size = App->GetSceneModule()->GetScene()->GetWindowSize();
    if (width != std::get<0>(size) || height != std::get<1>(size))
    {
        OnWindowResize(std::get<0>(size), std::get<1>(size));
    }
}

void CanvasComponent::Render(float deltaTime)
{
    if (!IsEffectivelyEnabled()) return;
}

void CanvasComponent::RenderDebug(float deltaTime)
{
    if (!IsEffectivelyEnabled()) return;

    App->GetDebugDrawModule()->DrawLine(
        float3(
            parent->GetGlobalTransform().TranslatePart().x - width / 2,
            parent->GetGlobalTransform().TranslatePart().y + height / 2, 0
        ),
        float3::unitX, width, float3(1, 1, 1)
    );
    App->GetDebugDrawModule()->DrawLine(
        float3(
            parent->GetGlobalTransform().TranslatePart().x - width / 2,
            parent->GetGlobalTransform().TranslatePart().y - height / 2, 0
        ),
        float3::unitX, width, float3(1, 1, 1)
    );

    App->GetDebugDrawModule()->DrawLine(
        float3(
            parent->GetGlobalTransform().TranslatePart().x - width / 2,
            parent->GetGlobalTransform().TranslatePart().y + height / 2, 0
        ),
        -float3::unitY, height, float3(1, 1, 1)
    );

    App->GetDebugDrawModule()->DrawLine(
        float3(
            parent->GetGlobalTransform().TranslatePart().x + width / 2,
            parent->GetGlobalTransform().TranslatePart().y + height / 2, 0
        ),
        -float3::unitY, height, float3(1, 1, 1)
    );
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

   const float4x4& view = isInWorldSpace ? App->GetCameraModule()->GetViewMatrix() : float4x4::identity;
    const float4x4& proj =
        isInWorldSpace ? App->GetCameraModule()->GetProjectionMatrix() : float4x4::D3DOrthoProjLH(-1, 1, width, height);

    for (const GameObject* child : sortedChildren)
    {
        if (!child->IsGloballyEnabled()) continue;

        // Only render UI components
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

        ImGui::Checkbox("Show in world space", &isInWorldSpace);
    }

    ImGui::SeparatorText("Canvas Scaling");

    // Select scale mode
    const char* scaleModeNames[] = {"Match Width", "Match Height", "Expand", "Shrink", "Match Width Or Height"};

    int currentMode              = static_cast<int>(scaleMode);
    if (ImGui::Combo("UI Scale Mode", &currentMode, scaleModeNames, IM_ARRAYSIZE(scaleModeNames)))
    {
        scaleMode = static_cast<UIScaleMode>(currentMode);
    }

    // Slider only visible if the mode is MatchWidthOrHeight
    if (scaleMode == UIScaleMode::MatchWidthOrHeight)
    {
        ImGui::SliderFloat("Match Factor", &matchFactor, 0.0f, 1.0f, "%.2f");
        ImGui::TextUnformatted("0 = Width, 1 = Height");
    }

    if (ImGui::Button("Debug Transform2D Hierarchy"))
    {
        PrintTransform2DDebugInfo();
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

    // Check play-mode before resize
    if (App->GetSceneModule()->GetInPlayMode())
    {
        for (const GameObject* child : sortedChildren)
        {
            if (Transform2DComponent* t2d = const_cast<GameObject*>(child)->GetComponent<Transform2DComponent*>())
            {
                t2d->AdaptToParentChangesRecursive();
            }
        }
    }

    if (uiScaler) delete uiScaler;
    uiScaler = new UIScaler(referenceWidth, referenceHeight, scaleMode, matchFactor);
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
    if (isInWorldSpace) return;

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

void CanvasComponent::PrintTransform2DDebugInfo() const
{
    GLOG("=== Transform2D Hierarchy Debug ===");

    for (const GameObject* child : sortedChildren)
    {
        if (Transform2DComponent* t2d = const_cast<GameObject*>(child)->GetComponent<Transform2DComponent*>())
        {
            PrintTransform2DRecursive(t2d, 0);
        }
    }

    GLOG("===================================");
}

void CanvasComponent::PrintTransform2DRecursive(Transform2DComponent* t2d, int depth) const
{
    std::string indent(depth * 2, ' ');
    const std::string& name = t2d->GetParent()->GetName(); // assuming GameObject::GetName()

    GLOG(
        "%s[%s] pos=(%.1f, %.1f) size=(%.1f, %.1f) pivot=(%.2f, %.2f) anchorsX=(%.2f, %.2f) anchorsY=(%.2f, %.2f) "
        "parent=%s",
        indent.c_str(), name.c_str(), t2d->GetLocalPosition().x, t2d->GetLocalPosition().y, t2d->GetSize().x,
        t2d->GetSize().y, t2d->GetPivot().x, t2d->GetPivot().y, t2d->GetAnchorsX().x, t2d->GetAnchorsX().y,
        t2d->GetAnchorsY().x, t2d->GetAnchorsY().y, t2d->HasParentTransform() ? "yes" : "no"
    );

    for (Transform2DComponent* child : t2d->GetChildTransforms())
    {
        PrintTransform2DRecursive(child, depth + 1);
    }
}


float CanvasComponent::GetUIScale() const
{
    return uiScaler ? uiScaler->GetScale(width, height) : 1.0f;
}
