#pragma once

#include "../ResourceManagement/Resources/ResourceTexture.h"
#include "Globals.h"

#include "Math/float3.h"
#include "Math/float4.h"
#include <memory>
#include <Libs/rapidjson/document.h>

class Component;

namespace Math
{
    class float4x4;
}

namespace Lights
{
    struct AmbientLightShaderData
    {
        float4 color;

        AmbientLightShaderData(const float4& color) : color(color) {}
    };

    struct DirectionalLightShaderData
    {
        float4 direction;
        float4 color;

        DirectionalLightShaderData(const float4& dir, const float4& color) : direction(dir), color(color) {}
    };

    struct PointLightShaderData
    {
        float4 position;
        float4 color;

        PointLightShaderData(const float4& pos, const float4& color) : position(pos), color(color) {}
    };

    struct SpotLightShaderData
    {
        float4 position;
        float4 color;
        float3 direction;
        float innerAngle;
        float outerAngle;

        SpotLightShaderData(
            const float4& pos, const float4& color, const float3& dir, const float inner, const float outer
        )
            : position(pos), color(color), direction(dir), innerAngle(inner), outerAngle(outer)
        {
        }
    };
} // namespace Lights

class DirectionalLightComponent;
class PointLightComponent;
class SpotLightComponent;

class LightsConfig
{
  public:
    LightsConfig();
    ~LightsConfig();

    void EditorParams();

    void InitSkybox();
    void RenderSkybox() const;

    void InitLightBuffers();
    void RenderLights() const;

    void AddDirectionalLight(DirectionalLightComponent* newDirectional);
    void AddPointLight(PointLightComponent* newPoint);
    void AddSpotLight(SpotLightComponent* newSpot);

    void RemoveDirectionalLight();
    void RemovePointLight(UID pointUid);
    void RemoveSpotLight(UID spotUid);

    void SaveData(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const;
    void LoadData(const rapidjson::Value& lights);

  private:
    unsigned int LoadSkyboxTexture(UID cubemapUID);

    void GetAllSceneLights();
    void GetAllPointLights(const std::unordered_map<UID, Component*>& components);
    void GetAllSpotLights(const std::unordered_map<UID, Component*>& components);
    void GetDirectionalLight(const std::unordered_map<UID, Component*>& components);

    void SetDirectionalLightShaderData() const;
    void SetPointLightsShaderData() const;
    void SetSpotLightsShaderData() const;

  private:
    UID skyboxUID;
    unsigned int skyboxVao;
    unsigned int skyboxTexture;
    unsigned int skyboxProgram;
    float3 ambientColor;
    float ambientIntensity;
    unsigned int directionalBufferId;
    unsigned int ambientBufferId;
    unsigned int pointBufferId;
    unsigned int spotBufferId;

    DirectionalLightComponent *directionalLight = nullptr;
    std::vector<PointLightComponent*> pointLights;
    std::vector<SpotLightComponent*> spotLights;

    ResourceTexture* currentTexture = nullptr;
    std::string currentTextureName = "Not selected";
};