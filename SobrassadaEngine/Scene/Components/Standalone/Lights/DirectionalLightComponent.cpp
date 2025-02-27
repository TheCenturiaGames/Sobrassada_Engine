#include "DirectionalLightComponent.h"
#include "Application.h"
#include "DebugDrawModule.h"
#include "SceneModule.h"

#include "Math/Quat.h"
#include "imgui.h"

DirectionalLightComponent::DirectionalLightComponent(UID uid, UID uidParent, UID uidRoot, const Transform& parentGlobalTransform)
    : LightComponent(uid, uidParent, uidRoot, "Directional Light", COMPONENT_DIRECTIONAL_LIGHT, parentGlobalTransform)
{
    LightsConfig* lightsConfig = App->GetSceneModule()->GetLightsConfig();
    if (lightsConfig != nullptr) lightsConfig->AddDirectionalLight(this);
}

DirectionalLightComponent::DirectionalLightComponent(const rapidjson::Value& initialState) : LightComponent(initialState)
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

    // Would be more optimal to only update the direction when rotation is modified
    float4x4 rot = float4x4::FromQuat(
        Quat::FromEulerXYZ(globalTransform.rotation.x, globalTransform.rotation.y, globalTransform.rotation.z)
    );
    DebugDrawModule* debug = App->GetDebugDrawModule();
    debug->DrawLine(globalTransform.position, (rot.RotatePart() * -float3::unitY).Normalized(), 2, float3(1, 1, 1));
}

float3 DirectionalLightComponent::GetDirection() const
{
    return (float4x4::FromQuat(
                Quat::FromEulerXYZ(globalTransform.rotation.x, globalTransform.rotation.y, globalTransform.rotation.z)
            )
                .RotatePart() *
            -float3::unitY)
        .Normalized();
}
