#pragma once

#include "CameraModule.h"
#include "Geometry/Frustum.h"
#include "Math/float4x4.h"
#include "Scene/Components/Component.h"
#include <Libs/rapidjson/document.h>

class CameraComponent : public Component
{
  public:
    CameraComponent(UID uid, UID uidParent, UID uidRoot, const float4x4& parentGlobalTransform);
    CameraComponent(const rapidjson::Value& initialState);
    ~CameraComponent();

    void Update() override;
    void Render() override;
    void RenderEditorInspector() override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;

    const FrustumPlanes& GetFrustrumPlanes() const { return frustumPlanes; }
    const float3& GetCameraPosition() const { return camera.pos; }
    unsigned int GetUbo() const { return ubo; }
    const float4x4& GetProjectionMatrix() { return camera.ProjectionMatrix(); }
    const float4x4& GetViewMatrix() { return camera.ViewMatrix(); }

  private:
    Frustum camera;
    FrustumPlanes frustumPlanes;
    CameraMatrices matrices;
    unsigned int ubo        = 0;
    bool drawGizmos         = false;
    bool isMainCamera       = false;

    float horizontalFov;
    float verticalFov;

    float orthographicWidth;
    float orthographicHeight;
};
