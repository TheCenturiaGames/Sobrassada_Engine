#pragma once

#include "Character.h"

#include <unordered_map>

class GameObject;
class AIAgentComponent;

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
    AIAgentComponent* agentAI = nullptr;
    std::unordered_map<std::string, SoldierStates> stateMap;
};