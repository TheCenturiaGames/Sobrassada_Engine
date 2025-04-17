#include "pch.h"

#include "Character.h"
#include "GameObject.h"

bool Character::Init()
{
    return true;
}

void Character::Update(float deltaTime)
{
}

void Character::Attack(float deltaTime)
{
    if (CanAttack(deltaTime)) PerformAttack();
}

bool Character::CanAttack(float deltaTime)
{
    deltaTime *= 1000.0f;
    if (deltaTime - lastAttackTime >= cooldown)
    {
        lastAttackTime = deltaTime;
        return true;
    }

    return false;
}

void Character::TakeDamage(int amount)
{
    currentHealth -= amount;

    if (currentHealth <= 0) Kill();
    else OnDamageTaken(amount);
}

void Character::Heal(int amount)
{
    currentHealth += amount;

    if (currentHealth > maxHealth) currentHealth = maxHealth;

    OnHealed(amount);
}

void Character::Kill()
{
    GLOG("Killed Character");
    isDead = true;
    // TODO: Disable GameObject
    OnDeath();
}
