#pragma once

#include "Character.h"

#include <unordered_map>

class GameObject;
class AIAgentComponent;
class AnimationComponent;

enum class SoldierStates
{
    NONE,
    PATROL,
    CHASE,
    BASIC_ATTACK
};

class Soldier : public Character
{
  public:
    Soldier(GameObject* parent);
    ~Soldier() noexcept override { parent = nullptr; };

    bool Init() override;
    void Update(float deltaTime) override;

  private:
    void OnDeath() override;
    void OnDamageTaken(int amount) override;
    void OnHealed(int amount) override;
    void PerformAttack() override;

    void HandleState(float deltaTime);
    void ChaseAI();

  private:
    AIAgentComponent* agentAI         = nullptr;
    AnimationComponent* animComponent = nullptr;
    std::unordered_map<std::string, SoldierStates> stateMap;
};