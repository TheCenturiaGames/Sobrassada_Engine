#pragma once

#include "Character.h"

class GameObject;
class AIAgentComponent;

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

  private:
    AIAgentComponent* agentAI = nullptr;
};