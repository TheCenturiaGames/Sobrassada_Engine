#pragma once

#include "Globals.h"
#include "Scene/Components/Component.h"
#include "AnimController.h"
#include <Libs/rapidjson/document.h>
#include <cstdint>

class AnimationComponent : public Component
{
  public:
    AnimationComponent(UID uid, GameObject* parent);
    AnimationComponent(const rapidjson::Value& initialState, GameObject* parent);
    
    ~AnimationComponent();
    void OnPlay();
    void OnStop();
    void OnUpdate();
    void OnInspector();

    void Update(float deltaTime) override;
    void Render(float deltaTime) override;
    void RenderEditorInspector() override;
  

    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;
    void AddAnimation(UID resource);
    UID GetAnimationResource() const { GLOG("Resource AnimUID  is: %d",resource) return resource; }
    void SetAnimationResource(UID animResource);
    void SetCurrentAnimation(ResourceAnimation* resourceAnim) { currentAnimResource = resourceAnim; }
    ResourceAnimation* GetCurrentAnimation() const { return currentAnimResource; }
  

  private:
    AnimController* animController = nullptr;
    ResourceAnimation* currentAnimResource        = nullptr;
    UID resource                   = INVALID_UID;
    std::string currentAnimName           = "None";
    GameObject* owner                     = nullptr;
  
};
