#include "DirectionalLightComponent.h"
#include "Application.h"
#include "DebugDrawModule.h"
#include "GameObject.h"
#include "SceneModule.h"

DirectionalLightComponent::DirectionalLightComponent(UID uid, UID uidParent)
    : LightComponent(uid, uidParent, "Directional Light", COMPONENT_DIRECTIONAL_LIGHT)
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
    if (!enabled || !drawGizmos || App->GetSceneModule()->GetInPlayMode()) return;

    DebugDrawModule* debug = App->GetDebugDrawModule();
    debug->DrawLine(
        GetParent()->GetGlobalTransform().TranslatePart(),
        (GetParent()->GetGlobalTransform().RotatePart() * -float3::unitY).Normalized(), 2, float3(1, 1, 1)
    );
}

const float3 DirectionalLightComponent::GetDirection()
{
    return (GetParent()->GetGlobalTransform().RotatePart() * -float3::unitY).Normalized();
}
