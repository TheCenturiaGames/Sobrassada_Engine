#include "DirectionalLightComponent.h"
#include "Application.h"
#include "DebugDrawModule.h"
#include "SceneModule.h"

#include "Math/Quat.h"
#include "imgui.h"

DirectionalLightComponent::DirectionalLightComponent(
    UID uid, UID uidParent, UID uidRoot, const float4x4& parentGlobalTransform
)
    : LightComponent(uid, uidParent, uidRoot, "Directional Light", COMPONENT_DIRECTIONAL_LIGHT, parentGlobalTransform)
{
    LightsConfig* lightsConfig = App->GetSceneModule()->GetLightsConfig();
    if (lightsConfig != nullptr) lightsConfig->AddDirectionalLight(this);
}

DirectionalLightComponent::DirectionalLightComponent(const rapidjson::Value& initialState)
    : LightComponent(initialState)
{
    LightsConfig* lightsConfig = App->GetSceneModule()->GetLightsConfig();
    if (lightsConfig != nullptr) lightsConfig->AddDirectionalLight(this);
}

DirectionalLightComponent::~DirectionalLightComponent()
{
    App->GetSceneModule()->GetLightsConfig()->RemoveDirectionalLight();
}

void DirectionalLightComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    LightComponent::Save(targetState, allocator);
}

void DirectionalLightComponent::Render()
{
    if (!enabled || !drawGizmos) return;

    DebugDrawModule* debug = App->GetDebugDrawModule();
    debug->DrawLine(
        globalTransform.TranslatePart(), (globalTransform.RotatePart() * -float3::unitY).Normalized(), 2,
        float3(1, 1, 1)
    );
}

float3 DirectionalLightComponent::GetDirection() const
{
    return (globalTransform.RotatePart() * -float3::unitY).Normalized();
}
