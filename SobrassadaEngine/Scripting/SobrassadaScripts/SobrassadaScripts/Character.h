#pragma once

#include "Script.h"

class GameObject;
class CharacterControllerComponent;

class Character : public Script
{
  public:
    Character(GameObject* parent, int maxHealth, int damage, float speed, float cooldown, float range)
        : Script(parent) {};

    virtual bool Init() override = 0;
    virtual void Update(float deltaTime) override;

    void CanAttack();
    void ShouldAttackTarget();
    void TakeDamage();
    void Heal(int amount);
    void Kill();

  protected:
    void SetMainCharacter(CharacterControllerComponent* mainCharacter) { cuChulainn = mainCharacter; };

  private:
    virtual void OnDeath() {};
    virtual void OnDamageTaken(int amount) {};
    virtual void OnHealed(int amount) {};
    virtual void PerformAttack() {};

  protected:
    CharacterControllerComponent* cuChulainn = nullptr;
    int maxHealth        = 0;
    int currentHealth    = 0;
    bool isInvulnerable  = false;
    bool isDead          = false;
    int damage           = 0;
    float speed          = 0.0f;
    float cooldown       = 0.0f;
    float lastAttackTime = 0.0f;
    float range          = 0.0f;
};
