#pragma once

#include "Character.h"

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
    ~Soldier() noexcept override { parent = nullptr; };

    bool Init() override;
    void Update(float deltaTime) override;

  private:
    void OnDeath() override;
    void OnDamageTaken(int amount) override;
    void PerformAttack() override;
    void HandleState(float deltaTime) override;

    void PatrolAI();
    void ChaseAI();

  private:
    AIAgentComponent* agentAI         = nullptr;
    SoldierStates currentState        = SoldierStates::NONE;
};