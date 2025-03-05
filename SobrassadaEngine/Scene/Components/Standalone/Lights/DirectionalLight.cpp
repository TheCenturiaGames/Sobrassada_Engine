#include "DirectionalLight.h"
#include "Application.h"
#include "DebugDrawModule.h"
#include "Math/Quat.h"
#include "SceneModule.h"
#include "imgui.h"

DirectionalLight::DirectionalLight(UID uid, UID uidParent, UID uidRoot, const float4x4& parentGlobalTransform)
    : LightComponent(uid, uidParent, uidRoot, "Directional Light", COMPONENT_DIRECTIONAL_LIGHT, parentGlobalTransform)
{
    direction                  = -float3::unitY;
    LightsConfig* lightsConfig = App->GetSceneModule()->GetLightsConfig();
    if (lightsConfig != nullptr) lightsConfig->AddDirectionalLight(this);
}

DirectionalLight::DirectionalLight(const rapidjson::Value& initialState) : LightComponent(initialState)
{
    direction                  = -float3::unitY;
    LightsConfig* lightsConfig = App->GetSceneModule()->GetLightsConfig();
    if (lightsConfig != nullptr) lightsConfig->AddDirectionalLight(this);
}

DirectionalLight::~DirectionalLight()
{
    App->GetSceneModule()->GetLightsConfig()->RemoveDirectionalLight();
}

void DirectionalLight::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    LightComponent::Save(targetState, allocator);
}

void DirectionalLight::Render()
{
    if (!enabled || !drawGizmos) return;

    // Would be more optimal to only update the direction when rotation is modified

    direction              = (globalTransform.RotatePart() * -float3::unitY).Normalized();

    DebugDrawModule* debug = App->GetDebugDrawModule();
    debug->DrawLine(globalTransform.TranslatePart(), direction, 2, float3(1, 1, 1));
}

void DirectionalLight::RenderEditorInspector()
{
    LightComponent::RenderEditorInspector();

    if (enabled)
    {
        ImGui::Text("Directional light parameters");
        ImGui::SliderFloat3("Direction ", &direction[0], -1.0, 1.0);
    }
}
