#pragma once

#include "Geometry/Frustum.h"
#include "Math/float4x4.h"
#include "Scene/Components/Component.h"
#include <Libs/rapidjson/document.h>
#include "CameraModule.h"

#define RADTODEG (180.0f / 3.14159265358979323846f)

class CameraComponent : public Component
{
  public:
    CameraComponent(UID uid, UID uidParent, UID uidRoot, const Transform& parentGlobalTransform);
    CameraComponent(const rapidjson::Value& initialState);
    ~CameraComponent();

    void Update() override;
    void Render() override;
    void RenderEditorInspector() override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;

    const FrustumPlanes& GetFrustrumPlanes() const { return frustumPlanes; }
    const float3& GetCameraPosition() const { return camera.pos; }
    unsigned int GetUbo() const { return ubo; }

  private:
    Frustum camera;
    FrustumPlanes frustumPlanes;
    CameraMatrices matrices;
    unsigned int ubo = 0;
    bool drawGizmos = false;
};
