#pragma once

#include "Script.h"

#include "Math/float3.h"
#include <vector>

class GameObject;
class CharacterControllerComponent;
class AnimationComponent;

class Character : public Script
{
  public:
    Character(GameObject* parent, int maxHealth, int damage, float speed, float cooldown, float range);
    virtual ~Character() noexcept override { parent = nullptr; };

    virtual bool Init() override;
    virtual void Update(float deltaTime) override;

  protected:
    void Attack(float deltaTime);
    void TakeDamage(int amount);
    void Heal(int amount);

  private:
    virtual void OnDeath() {};
    virtual void OnDamageTaken(int amount) {}; // depending of amout damage taken do some sound or another for example
    virtual void OnHealed(int amount) {};
    virtual void PerformAttack() {};
    virtual void ShouldAttackTarget() {};
    bool CanAttack(float deltaTime);
    void Die();

  protected:
    int maxHealth                     = 0;
    int currentHealth                 = 0;
    bool isInvulnerable               = false;
    bool isDead                       = false;
    int damage                        = 0;
    float speed                       = 0.0f;
    float cooldown                    = 0.0f;
    float lastAttackTime              = -1.0f;
    float range                       = 0.0f;
    AnimationComponent* animComponent = nullptr;
};
