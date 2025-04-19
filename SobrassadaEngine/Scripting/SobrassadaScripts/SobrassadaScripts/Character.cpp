#include "pch.h"

#include "Character.h"
#include "GameObject.h"
#include "Standalone/AnimationComponent.h"

Character::Character(GameObject* parent, int maxHealth, int damage, float speed, float cooldown, float range)
    : Script(parent), maxHealth(maxHealth), damage(damage), speed(speed), cooldown(cooldown), range(range)
{
    currentHealth      = maxHealth;
}

bool Character::Init()
{
    animComponent = parent->GetAnimationComponent();
    if (!animComponent)
    {
        GLOG("Animation component not found for %s", parent->GetName().c_str());
        return false;
    }
    animComponent->OnPlay(false);

    return true;
}

void Character::Update(float deltaTime)
{
}

void Character::Attack(float deltaTime)
{
    if (CanAttack(deltaTime)) PerformAttack();
}

void Character::TakeDamage(int amount)
{
    currentHealth -= amount;

    if (currentHealth <= 0) Die();
    else OnDamageTaken(amount);
}

void Character::Heal(int amount)
{
    currentHealth += amount;

    if (currentHealth > maxHealth) currentHealth = maxHealth;

    OnHealed(amount);
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

void Character::Die()
{
    GLOG("%s dead", parent->GetName().c_str());
    isDead = true;
    // TODO: Disable GameObject
    OnDeath();
}
