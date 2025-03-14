#pragma once

#include "CameraModule.h"
#include "Geometry/Frustum.h"
#include "Math/float4x4.h"
#include "Scene/Components/Component.h"
#include <Libs/rapidjson/document.h>

class CameraComponent : public Component
{
  public:
    CameraComponent(UID uid, UID uidParent);
    CameraComponent(const rapidjson::Value& initialState);
    ~CameraComponent() override;

    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;

    void Update() override;
    void Render() override;
    void RenderEditorInspector() override;

    void SetAspectRatio(float newAspectRatio);
    void ChangeToPerspective();
    void ChangeToOrtographic();

    const FrustumPlanes& GetFrustrumPlanes() const { return frustumPlanes; }
    const float3& GetCameraPosition() const { return camera.pos; }
    unsigned int GetUbo() const { return ubo; }
    const float4x4 GetProjectionMatrix() { return camera.ProjectionMatrix(); }
    const float4x4 GetViewMatrix() { return camera.ViewMatrix(); }
    const int GetType() { return (camera.type == OrthographicFrustum) ? 1 : 0; }

  private:
    Frustum camera;
    FrustumPlanes frustumPlanes;
    CameraMatrices matrices;
    unsigned int ubo  = 0;

    bool drawGizmos   = true;
    bool isMainCamera = false;

    float horizontalFov;
    float verticalFov;
    float perspectiveNearPlane = 0.1f;
    float perspectiveFarPlane  = 50.0f;

    float orthographicWidth;
    float orthographicHeight;
    float ortographicNearPlane = 25.10f;
    float ortographicFarPlane  = 50.0f;

    bool firstTime             = true;
};
