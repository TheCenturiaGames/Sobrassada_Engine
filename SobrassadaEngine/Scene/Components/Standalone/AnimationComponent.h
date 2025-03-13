#pragma once

#include "Globals.h"
#include "Scene/Components/Component.h"
#include "AnimController.h"
#include <Libs/rapidjson/document.h>
#include <cstdint>

class AnimationComponent : public Component
{
  public:
    AnimationComponent(UID uid, UID uidParent);
    AnimationComponent(const rapidjson::Value& initialState);
    ~AnimationComponent();
    void OnPlay();
    void OnStop();
    void OnUpdate();
    void OnInspector();
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void AddAnimation(UID resource);
    void SetAnimationResource(UID animResource);
    UID GetAnimationResource() const { return resource; }


  private:
    AnimController* animController = nullptr;
    ResourceAnimation* anim        = nullptr;
    UID resource;

};
