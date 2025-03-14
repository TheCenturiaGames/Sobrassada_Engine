#include "PointLightComponent.h"

#include "Application.h"
#include "DebugDrawModule.h"
#include "GameObject.h"
#include "SceneModule.h"

#include "ImGui.h"
#include <vector>

PointLightComponent::PointLightComponent(UID uid, UID uidParent)
    : LightComponent(uid, uidParent, "Point Light", COMPONENT_POINT_LIGHT)
{
    range                      = 1;
    gizmosMode                 = 0;

    LightsConfig* lightsConfig = App->GetSceneModule()->GetLightsConfig();
    if (lightsConfig != nullptr) lightsConfig->AddPointLight(this);
}

PointLightComponent::PointLightComponent(const rapidjson::Value& initialState) : LightComponent(initialState)
{

    if (initialState.HasMember("Range"))
    {
        range = initialState["Range"].GetFloat();
    }
    if (initialState.HasMember("GizmosMode"))
    {
        gizmosMode = initialState["GizmosMode"].GetInt();
    }
    LightsConfig* lightsConfig = App->GetSceneModule()->GetLightsConfig();
    if (lightsConfig != nullptr) lightsConfig->AddPointLight(this);
}

PointLightComponent::~PointLightComponent()
{
    App->GetSceneModule()->GetLightsConfig()->RemovePointLight(this);
}

void PointLightComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    LightComponent::Save(targetState, allocator);

    targetState.AddMember("Range", range, allocator);
    targetState.AddMember("GizmosMode", gizmosMode, allocator);
}

void PointLightComponent::Clone(const Component* other)
{
    if (other->GetType() == ComponentType::COMPONENT_POINT_LIGHT)
    {
        const PointLightComponent* otherLight = static_cast<const PointLightComponent*>(other);
        enabled                              = otherLight->enabled;

        intensity                             = otherLight->intensity;
        color                                 = otherLight->color;
        drawGizmos                            = otherLight->drawGizmos;

        range                                = otherLight->range;
        gizmosMode                            = otherLight->gizmosMode;
    }
    else
    {
        GLOG("It is not possible to clone a component of a different type!");
    }
}

void PointLightComponent::RenderEditorInspector()
{
    LightComponent::RenderEditorInspector();

    if (enabled)
    {
        ImGui::Text("Point light parameters");
        ImGui::SliderFloat("Range", &range, 0.0f, 10.0f);

        ImGui::Text("Gizmos mode");
        if (ImGui::RadioButton("Lines", &gizmosMode, 0))
        {
        }
        ImGui::SameLine();
        if (ImGui::RadioButton("Sphere", &gizmosMode, 1))
        {
        }
    }
}

void PointLightComponent::Render()
{
    if (!enabled || !drawGizmos || App->GetSceneModule()->GetInPlayMode()) return;

    GLOG("%d", parentUID);

    std::vector<float3> directions;
    directions.push_back(float3::unitX);
    directions.push_back(float3::unitY);
    directions.push_back(float3::unitZ);
    directions.push_back(-float3::unitX);
    directions.push_back(-float3::unitY);
    directions.push_back(-float3::unitZ);

    directions.push_back(float3(1, 0, 1).Normalized());
    directions.push_back(float3(-1, 0, 1).Normalized());
    directions.push_back(float3(-1, 0, -1).Normalized());
    directions.push_back(float3(1, 0, -1).Normalized());

    directions.push_back(float3(1, 1, 0).Normalized());
    directions.push_back(float3(-1, 1, 0).Normalized());
    directions.push_back(float3(-1, -1, 0).Normalized());
    directions.push_back(float3(1, -1, 0).Normalized());

    directions.push_back(float3(0, 1, 1).Normalized());
    directions.push_back(float3(0, 1, -1).Normalized());
    directions.push_back(float3(0, -1, -1).Normalized());
    directions.push_back(float3(0, -1, 1).Normalized());

    directions.push_back(float3(1, 1, 1).Normalized());
    directions.push_back(float3(1, -1, 1).Normalized());
    directions.push_back(float3(-1, 1, 1).Normalized());
    directions.push_back(float3(-1, -1, 1).Normalized());
    directions.push_back(float3(-1, 1, -1).Normalized());
    directions.push_back(float3(-1, -1, -1).Normalized());
    directions.push_back(float3(1, 1, -1).Normalized());
    directions.push_back(float3(1, -1, -1).Normalized());

    DebugDrawModule* debug = App->GetDebugDrawModule();

    if (gizmosMode == 0)
    {
        for (int i = 0; i < directions.size(); ++i)
        {
            debug->DrawLine(GetParent()->GetGlobalTransform().TranslatePart(), directions[i], range, float3(1, 1, 1));
        }
    }
    else
    {
        debug->DrawSphere(GetParent()->GetGlobalTransform().TranslatePart(), float3(1, 1, 1), range);
    }
}