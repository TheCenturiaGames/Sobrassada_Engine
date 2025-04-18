#pragma once

#include "Character.h"
#include <unordered_map>

class GameObject;
class CharacterControllerComponent;
class AnimationComponent;

enum class CharacterStates
{
    NONE,
    IDLE,
    RUN,
    DASH,
    BASIC_ATTACK
};

class CuChulainn : public Character
{
  public:
    CuChulainn(GameObject* parent);
    virtual ~CuChulainn() noexcept override { parent = nullptr; };

    bool Init() override;
    void Update(float deltaTime) override;

  private:
    void OnDeath() override;
    void OnDamageTaken(int amount) override;
    void OnHealed(int amount) override;
    void PerformAttack() override;

    void HandleAnimation();

  private:
    std::unordered_map<std::string, CharacterStates> stateMap;
    AnimationComponent* animComponent = nullptr;
    bool runActive = false;
};

extern CharacterControllerComponent* character;
