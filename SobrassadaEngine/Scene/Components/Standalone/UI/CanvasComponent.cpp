#include "CanvasComponent.h"

#include "Application.h"
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
    width                = initialState["Width"].GetFloat();
    height               = initialState["Height"].GetFloat();
    isInWorldSpaceEditor = initialState["IsInWorldSpaceEditor"].GetBool();
    isInWorldSpaceGame   = initialState["IsInWorldSpaceGame"].GetBool();
}

CanvasComponent::~CanvasComponent()
{
    App->GetGameUIModule()->RemoveCanvas(this);
}

void CanvasComponent::Init()
{
    App->GetGameUIModule()->AddCanvas(this);

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

void CanvasComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);

    targetState.AddMember("Width", width, allocator);
    targetState.AddMember("Height", height, allocator);
    targetState.AddMember("IsInWorldSpaceEditor", isInWorldSpaceEditor, allocator);
    targetState.AddMember("IsInWorldSpaceGame", isInWorldSpaceGame, allocator);
}

void CanvasComponent::Clone(const Component* other)
{
    if (other->GetType() == ComponentType::COMPONENT_CANVAS)
    {
        const CanvasComponent* otherCanvas = static_cast<const CanvasComponent*>(other);
        width                              = otherCanvas->width;
        height                             = otherCanvas->height;

        isInWorldSpaceEditor               = otherCanvas->isInWorldSpaceEditor;
        isInWorldSpaceGame                 = otherCanvas->isInWorldSpaceGame;
    }
    else
    {
        GLOG("It is not possible to clone a component of a different type!");
    }
}

void CanvasComponent::Update(float deltaTime)
{
    if (!IsEffectivelyEnabled()) return;
}

void CanvasComponent::Render(float deltaTime)
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
    if (!parent->IsGloballyEnabled()) return;

    const int uiProgram = App->GetShaderModule()->GetUIWidgetProgram();
    if (uiProgram == -1)
    {
        GLOG("Error with UI Program");
        return;
    }
    glUseProgram(uiProgram);

    const float4x4& view = isInWorldSpaceEditor ? App->GetCameraModule()->GetViewMatrix() : float4x4::identity;
    const float4x4& proj = isInWorldSpaceEditor ? App->GetCameraModule()->GetProjectionMatrix()
                                                : float4x4::D3DOrthoProjLH(
                                                      -1, 1, (float)App->GetWindowModule()->GetWidth(),
                                                      (float)App->GetWindowModule()->GetHeight()
                                                  );

    std::queue<UID> children;

    for (const UID child : parent->GetChildren())
    {
        children.push(child);
    }

    while (!children.empty())
    {
        const GameObject* currentObject = App->GetSceneModule()->GetScene()->GetGameObjectByUID(children.front());
        children.pop();

        if (!currentObject->IsGloballyEnabled()) continue;

        if (currentObject->GetComponentByType(COMPONENT_TRANSFORM_2D))
        {
            Component* component = nullptr;

            component = currentObject->GetComponentByType(COMPONENT_TRANSFORM_2D);
            if (component) static_cast<const Transform2DComponent*>(component)->RenderWidgets();

            component = currentObject->GetComponentByType(COMPONENT_LABEL);
            if (component) static_cast<const UILabelComponent*>(component)->RenderUI(view, proj);

            component = currentObject->GetComponentByType(COMPONENT_IMAGE);
            if (component) static_cast<const ImageComponent*>(component)->RenderUI(view, proj);
        }

        for (const UID child : currentObject->GetChildren())
        {
            children.push(child);
        }
    }
}

void CanvasComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();

    if (enabled)
    {
        ImGui::Text("Canvas");

        ImGui::Checkbox("Show in world space", &isInWorldSpaceEditor);
    }
}

void CanvasComponent::OnWindowResize(const unsigned int width, const unsigned int height)
{
    this->width        = (float)width;
    this->height       = (float)height;

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

    parent->UpdateTransformForGOBranch();
}
