#pragma once

#include "Character.h"

class GameObject;
class CharacterControllerComponent;

class CuChulainn : public Character
{
  public:
    CuChulainn(GameObject* parent);

    bool Init() override;
    void Update(float deltaTime) override;

  private:
    void OnDeath() override;
    void OnDamageTaken(int amount) override;
    void OnHealed(int amount) override;
    void PerformAttack() override;
};

extern CharacterControllerComponent* character;
