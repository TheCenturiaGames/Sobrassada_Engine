#pragma once

#include "Geometry/Frustum.h"
#include "Math/float4x4.h"
#include "Scene/Components/Component.h"
#include <Libs/rapidjson/document.h>
#include "CameraModule.h"

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
    unsigned int GetUbo() const { return ubo; }

  private:
    Frustum camera;
    CameraMatrices matrices;
    unsigned int ubo;
};
