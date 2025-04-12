#pragma once

#include "Component.h"
#include "Globals.h"

#include "rapidjson/document.h"
#include <cstdint>
#include <list>
#include <unordered_map>

class ResourceAnimation;
class ResourceStateMachine;
class AnimController;
class GameObject;

struct ActiveAnimInfo
{
    ResourceAnimation* animation = nullptr;
    float currentTime            = 0.0f;
    float fadeTime               = 0.0f;
    bool looping                 = false;
};

class AnimationComponent : public Component
{
  public:
    AnimationComponent(UID uid, GameObject* parent);
    AnimationComponent(const rapidjson::Value& initialState, GameObject* parent);
    ~AnimationComponent() override;

    void Clone(const Component* other) override;
    void Update(float deltaTime) override;
    void Render(float deltaTime) override;
    void RenderEditorInspector() override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const override;

    void OnPlay();
    void OnStop();
    void OnPause();
    void OnResume();
    void OnInspector();
    void AddAnimation(UID resource);

    UID GetAnimationResource() const { GLOG("Resource AnimUID  is: %d", resource) return resource; }
    ResourceAnimation* GetCurrentAnimation() const { return currentAnimResource; }
    AnimController* GetAnimationController() { return animController; }
    std::unordered_map<std::string, GameObject*> GetBoneMapping() const { return boneMapping; }

    void SetAnimationResource(UID animResource);
    bool IsPlaying();

    void BlendAnimations();
    void PlayAnimation(ResourceAnimation* anim, bool loop);

  private:
    void SetBoneMapping();

  private:
    UID resource                               = INVALID_UID;
    std::string currentAnimName                = "None";
    AnimationComponent* currentAnimComp        = nullptr;

    AnimController* animController             = nullptr;
    ResourceAnimation* currentAnimResource     = nullptr;
    ResourceStateMachine* resourceStateMachine = nullptr;

    std::list<ActiveAnimInfo> activeAnimations;

    GameObject* owner = nullptr;
    std::unordered_map<std::string, GameObject*> boneMapping;

    float animationDuration = 0.0f;
    bool playing            = false;
    float currentTime       = 0.0f;
    float transitionTime    = 250;
    float fadeTime          = 0;
};