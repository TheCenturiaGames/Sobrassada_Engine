#pragma once

#include "Script.h"

class GameObject;
class CharacterControllerComponent;

class Character : public Script
{
  public:
    Character(GameObject* parent, int maxHealth, int damage, float speed, float cooldown, float range);
    virtual ~Character() noexcept override { parent = nullptr; };

    virtual bool Init() override;
    virtual void Update(float deltaTime) override;

    void Attack(float deltaTime);
    void TakeDamage(int amount);
    void Heal(int amount);
    void Kill();

  private:
    virtual void OnDeath() {};
    virtual void OnDamageTaken(int amount) {}; // depending of amout damage taken do some sound or another for example
    virtual void OnHealed(int amount) {};
    virtual void PerformAttack() {};
    virtual void ShouldAttackTarget() {};
    bool CanAttack(float deltaTime);

  protected:
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
