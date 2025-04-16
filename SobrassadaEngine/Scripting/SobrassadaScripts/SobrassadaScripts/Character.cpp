#include "pch.h"

#include "Character.h"
#include "GameObject.h"

void Character::Update(float deltaTime)
{

}

void Character::CanAttack()
{
    if (cooldown > 0.0f) return;

    //TODO
}

void Character::ShouldAttackTarget()
{
}

void Character::TakeDamage(int amount)
{
    currentHealth -= amount;

    if (currentHealth <= 0) Kill();
}

void Character::Heal(int amount)
{
    currentHealth += amount;

    if (currentHealth > maxHealth) currentHealth = maxHealth;
}

void Character::Kill()
{
    GLOG("Killed Character");
    isDead = true;
}
