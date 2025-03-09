#pragma once

#include "Globals.h"
#include "Scene/Components/Component.h"
#include "AnimController.h"

#include <Libs/rapidjson/document.h>
#include <cstdint>

class AnimationComponent : public Component
{
  public:
    AnimationComponent(UID uid, UID uidParent, UID uidRoot, const float4x4& parentGlobalTransform);
    AnimationComponent(const rapidjson::Value& initialState);

    void OnPlay();
    void OnStop();
    void OnUpdate();


  private:
    AnimController* animController = nullptr;
    UID resource;

};
