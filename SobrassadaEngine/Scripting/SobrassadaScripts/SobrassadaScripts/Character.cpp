#include "pch.h"

#include "Application.h"
#include "Character.h"
#include "CuChulainn.h"
#include "EditorUIModule.h"
#include "GameObject.h"
#include "GameTimer.h"
#include "Projectile.h"
#include "ScriptComponent.h"
#include "Standalone/AnimationComponent.h"
#include "Standalone/CharacterControllerComponent.h"
#include "Standalone/Physics/CapsuleColliderComponent.h"
#include "Standalone/Physics/CubeColliderComponent.h"

#include <string>

Character::Character(
    GameObject* parent, int maxHealth, int damage, float attackDuration, float speed, float cooldown, float range,
    float rangeAIAttack, float rangeAIChase
)
    : Script(parent), maxHealth(maxHealth), damage(damage), attackDuration(attackDuration), speed(speed),
      attackCooldown(cooldown), range(range), rangeAIAttack(rangeAIAttack), rangeAIChase(rangeAIChase)
{
    currentHealth = maxHealth;

    fields.push_back({"Max Health", InspectorField::FieldType::Int, &this->maxHealth, 0, 10});
    fields.push_back({"Current Health", InspectorField::FieldType::Int, &currentHealth, 0, 10});
    fields.push_back({"Invulnerable", InspectorField::FieldType::Bool, &isInvulnerable, true, false});
    fields.push_back({"Dead", InspectorField::FieldType::Bool, &isDead, true, false});
    fields.push_back({"Damage", InspectorField::FieldType::Int, &this->damage, 0, 3});
    fields.push_back({"Attack Duration", InspectorField::FieldType::Float, &this->attackDuration, 0.0f, 1.0f});
    fields.push_back({"Attack Cooldown", InspectorField::FieldType::Float, &attackCooldown, 0.0f, 2.0f});
    fields.push_back({"Attack Range", InspectorField::FieldType::Float, &this->range, 0.0f, 3.0f});
}

bool Character::Init()
{
    animComponent = parent->GetComponent<AnimationComponent*>();
    if (!animComponent)
    {
        GLOG("Animation component not found for %s", parent->GetName().c_str());
    }
    else
    {
        animComponent->OnPlay(false);
    }

    characterCollider = parent->GetComponent<CapsuleColliderComponent*>();
    if (!characterCollider)
    {
        GLOG("Character capsule collider component not found for %s", parent->GetName().c_str());
        return false;
    }

    weaponCollider = parent->GetComponentChild<CubeColliderComponent*>(AppEngine);
    if (!weaponCollider)
    {
        GLOG("Weapon cube collider component not found for %s", parent->GetName().c_str());
        return false;
    }
    weaponCollider->SetEnabled(false);

    lastAttackTime = -1.0f;
    lastTimeHit    = -1.0f;

    return true;
}

void Character::Update(float deltaTime)
{
    if (isDead) return;

    float gameTime = AppEngine->GetGameTimer()->GetTime() / 1000.0f;
    if (isAttacking && gameTime - lastAttackTime >= attackDuration)
    {
        // GLOG("Not Attacking %.3f", gameTime);
        if (weaponCollider && weaponCollider->GetEnabled()) weaponCollider->SetEnabled(false);
        isAttacking = false;
    }
    if (isInvulnerable && gameTime - lastTimeHit >= invulnerableDuration)
    {
        // GLOG("Not vulnerable %.3f", gameTime);
        isInvulnerable = false;
    }

    HandleState(gameTime);
}

void Character::OnCollision(GameObject* otherObject, const float3& collisionNormal)
{
    // cube collider should be only if is enabled here already checked by OnCollision of cubeColliderComponent
    // GLOG("COLLISION %s with %s", parent->GetName().c_str(), otherObject->GetName().c_str())

    // Melee check
    CubeColliderComponent* otherWeapon = otherObject->GetComponent<CubeColliderComponent*>();
    ScriptComponent* otherScript       = otherObject->GetComponentParent<ScriptComponent*>(AppEngine);

    if (isInvulnerable) return;

    if (otherScript && otherWeapon && otherWeapon->GetEnabled())
    {
        Character* enemyScript = otherScript->GetScriptByType<Character>();
        if (enemyScript)
        {
            if (!enemyScript->isAttacking) return;
            TakeDamage(enemyScript->damage);
        }
    }

    // Projectile check
    otherScript = otherObject->GetComponent<ScriptComponent*>();

    if (otherScript)
    {
        Projectile* projectile = otherScript->GetScriptByType<Projectile>();
        if (projectile && otherWeapon && otherWeapon->GetEnabled())
        {
            TakeDamage(projectile->GetDamage());
            otherWeapon->SetEnabled(false);
            otherObject->SetEnabled(false);
        }
    }
}

void Character::Attack(float time)
{
    if (CanAttack(time))
    {
        // GLOG("ATTACK");
        isAttacking    = true;
        lastAttackTime = time;

        // TODO: The enable and disable of the collider should be managed by each palyer and enemy,
        // depending on the timings of their attack animations as we don't have Animation Events (I think)
        if (weaponCollider) weaponCollider->SetEnabled(true);
        PerformAttack();
    }
}

void Character::TakeDamage(int amount)
{
    currentHealth      -= amount;

    isInvulnerable      = true;
    float newDeltaTime  = AppEngine->GetGameTimer()->GetTime() / 1000.0f;
    lastTimeHit         = newDeltaTime;

    if (currentHealth <= 0) Die();
    else OnDamageTaken(amount);
}

void Character::Heal(int amount)
{
    currentHealth += amount;

    if (currentHealth > maxHealth) currentHealth = maxHealth;

    OnHealed(amount);
}

bool Character::CanAttack(float time)
{
    if (!isAttacking && time - lastAttackTime >= attackCooldown) return true;

    return false;
}

AIStates Character::CheckDistanceWithPlayer() const
{
    if (character != nullptr)
    {
        float distance = character->GetLastPosition().Distance(parent->GetPosition());
        if (distance <= rangeAIAttack) return CLOSE;
        else if (distance <= rangeAIChase) return MEDIUM;
    }
    return FAR_AWAY;
}

void Character::Die()
{
    GLOG("%s dead", parent->GetName().c_str());
    isDead = true;
    OnDeath();

    if (characterCollider)
    {
        characterCollider->DeleteRigidBody();
        characterCollider->SetEnabled(false);
    }

    if (weaponCollider)
    {
        weaponCollider->DeleteRigidBody();
        weaponCollider->SetEnabled(false);
    }

    parent->SetEnabled(false);
}
