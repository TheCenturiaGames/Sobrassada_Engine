#pragma once
#include "Script.h"
#include <unordered_map>
#include <string>

enum class CharacterStates
{
    NONE,
    IDLE,
    RUN,
    DASH,
    BASIC_ATTACK
};

class CharacterScript : public Script
{
  public:
    CharacterScript(GameObject* parent);
    bool Init() override;
    void Update(float deltaTime) override;
    
    void Kill();
    void TakeDamage(int amount);
    void Heal(int amount);

    void HandleAnimation();

private:
    static std::unordered_map<std::string, CharacterStates> stateMap;

protected:
    int maxHealth;
    int currentHealth;
    bool isDead = false;
    int damage;
    float speed;
    float cooldown;
    float lastAttackTime;
    float range;
};
