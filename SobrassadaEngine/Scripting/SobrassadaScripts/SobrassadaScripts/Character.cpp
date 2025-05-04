#include "pch.h"

#include "Application.h"
#include "Character.h"
#include "CuChulainn.h"
#include "EditorUIModule.h"
#include "GameObject.h"
#include "GameTimer.h"
#include "ScriptComponent.h"
#include "Standalone/AnimationComponent.h"
#include "Standalone/CharacterControllerComponent.h"
#include "Standalone/Physics/CapsuleColliderComponent.h"
#include "Standalone/Physics/CubeColliderComponent.h"

#include <string>

Character::Character(
    GameObject* parent, int maxHealth, int damage, float attackDuration, float cooldown, float range,
    float rangeAIAttack, float rangeAIChase, const std::vector<float3>& patrolPoints
)
    : Script(parent)
{
    currentHealth = maxHealth;
    type          = CharacterType::None;

    // Using ImGui in the dll cause problems, so we need to call ImGui outside the dll
    fields.push_back({"Max Health", InspectorField::FieldType::Int, &maxHealth, 0, 10});
    fields.push_back({"Current Health", InspectorField::FieldType::Int, &currentHealth, 0, 10});
    fields.push_back({"Invulnerable", InspectorField::FieldType::Bool, &isInvulnerable, true, false});
    fields.push_back({"Dead", InspectorField::FieldType::Bool, &isDead, true, false});
    fields.push_back({"Damage", InspectorField::FieldType::Int, &damage, 0, 3});
    fields.push_back({"Attack Duration", InspectorField::FieldType::Float, &attackDuration, 0.0f, 1.0f});
    fields.push_back({"Attack Cooldown", InspectorField::FieldType::Float, &cooldown, 0.0f, 2.0f});
    fields.push_back({"Attack Range", InspectorField::FieldType::Float, &range, 0.0f, 3.0f});

    if (type != CharacterType::CuChulainn)
    {
        fields.push_back({"AI Chase Range", InspectorField::FieldType::Float, &rangeAIChase, 0.0f, 20.0f});
        fields.push_back({"AI Attack Range", InspectorField::FieldType::Float, &rangeAIAttack, 0.0f, 5.0f});

        fields.push_back({"AI Patrol Points", InspectorField::FieldType::Int, &patrolPointsCont, 0, 5});
        for (int i = 0; i < patrolPointsCont; i++)
        {
            std::string fieldName = std::to_string(i) + " Patrol Point";
            fields.push_back({fieldName.c_str(), InspectorField::FieldType::Vec3, &patrolPoints[i]});
        }
    }
}

bool Character::Init()
{
    animComponent = parent->GetComponent<AnimationComponent*>();
    if (animComponent == nullptr) GLOG("Animation component not found for %s", parent->GetName().c_str())
    else animComponent->OnPlay(false);

    characterCollider = parent->GetComponent<CapsuleColliderComponent*>();
    if (characterCollider == nullptr)
        GLOG("Character capsule collider component not found for %s", parent->GetName().c_str())

    weaponCollider = parent->GetComponentChild<CubeColliderComponent*>(AppEngine);
    if (weaponCollider == nullptr) GLOG("Weapon cube collider component not found for %s", parent->GetName().c_str())
    else weaponCollider->SetEnabled(false);

    lastAttackTime = -1.0f;
    lastTimeHit    = -1.0f;

    return true;
}

void Character::Update(float deltaTime)
{
    if (isDead) return;

    if (characterCollider == nullptr || weaponCollider == nullptr) return;

    float gameTime = AppEngine->GetGameTimer()->GetTime() / 1000.0f;
    if (isAttacking && weaponCollider->GetEnabled() && gameTime - lastAttackTime >= attackDuration)
    {
        // GLOG("Not Attacking %.3f", gameTime);
        weaponCollider->SetEnabled(false);
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

    CubeColliderComponent* enemyWeapon    = otherObject->GetComponent<CubeColliderComponent*>();

    ScriptComponent* enemyScriptComponent = otherObject->GetComponentParent<ScriptComponent*>(AppEngine);

    if (!isInvulnerable && enemyScriptComponent != nullptr && enemyWeapon != nullptr && enemyWeapon->GetEnabled())
    {
        Script* enemyScript = enemyScriptComponent->GetScriptInstance();

        if (enemyScript == nullptr)
            GLOG("%s script instance not setted.", otherObject->GetName().c_str()) // we want to crash?

        // ScriptType scriptType = enemyScriptComponent->GetScriptType(); // type of script for casting
        Character* enemyCharacter = dynamic_cast<Character*>(enemyScript);
        if (!enemyCharacter->isAttacking) return;

        TakeDamage(enemyCharacter->damage);
        isInvulnerable     = true;
        float newDeltaTime = AppEngine->GetGameTimer()->GetTime() / 1000.0f;
        lastTimeHit        = newDeltaTime;
    }
}

void Character::Attack(float time)
{
    if (CanAttack(time))
    {
        // GLOG("ATTACK");
        isAttacking    = true;
        lastAttackTime = time;
        weaponCollider->SetEnabled(true);
        PerformAttack();
    }
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

bool Character::CanAttack(float time)
{
    if (!isAttacking && time - lastAttackTime >= cooldown) return true;

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

    if (characterCollider != nullptr)
    {
        characterCollider->DeleteRigidBody();
        characterCollider->SetEnabled(false);
    }

    if (weaponCollider != nullptr)
    {
        weaponCollider->DeleteRigidBody();
        weaponCollider->SetEnabled(false);
    }

    parent->SetEnabled(false);
}
