#include "pch.h"

#include "Application.h"
#include "Character.h"
#include "EditorUIModule.h"
#include "GameObject.h"
#include "Standalone/AnimationComponent.h"
#include "Standalone/Physics/CapsuleColliderComponent.h"
#include "Standalone/Physics/CubeColliderComponent.h"

Character::Character(
    GameObject* parent, int maxHealth, int damage, float attackDuration, float speed, float cooldown, float range
)
    : Script(parent), maxHealth(maxHealth), damage(damage), attackDuration(attackDuration), speed(speed),
      cooldown(cooldown), range(range)
{
    currentHealth = maxHealth;
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

    characterCollider = dynamic_cast<CapsuleColliderComponent*>(parent->GetComponentByType(COMPONENT_CAPSULE_COLLIDER));
    if (!characterCollider)
    {
        GLOG("Character capsule collider component not found for %s", parent->GetName().c_str());
        return false;
    }

    weaponCollider = dynamic_cast<CubeColliderComponent*>(parent->GetComponentChildByType(COMPONENT_CUBE_COLLIDER));
    if (!weaponCollider)
    {
        GLOG("Weapon cube collider component not found for %s", parent->GetName().c_str());
        return false;
    }
    weaponCollider->SetEnabled(false);

    lastAttackTime = -1.0f;

    return true;
}

void Character::Update(float deltaTime)
{
    if (isDead) return;

    if (deltaTime - lastAttackTime >= attackDuration) weaponCollider->SetEnabled(false);

    HandleState(deltaTime);
}

void Character::Inspector()
{
    // Using ImGui in the dll cause problems, so we need to call ImGui outside the dll
    if (fields.empty())
    {
        fields.push_back({"Max Health", InspectorField::FieldType::Int, &maxHealth, 0, 10});
        fields.push_back({"Current Health", InspectorField::FieldType::Int, &currentHealth, 0, 10});
        fields.push_back({"Invulnerable", InspectorField::FieldType::Bool, &isInvulnerable, true, false});
        fields.push_back({"Dead", InspectorField::FieldType::Bool, &isDead, true, false});
        fields.push_back({"Damage", InspectorField::FieldType::Int, &damage, 0, 3});
        fields.push_back({"Attack Duration", InspectorField::FieldType::Float, &attackDuration, 0.0f, 1.0f});
        fields.push_back({"Speed", InspectorField::FieldType::Float, &speed, 0.0f, 10.0f});
        fields.push_back({"Attack Cooldown", InspectorField::FieldType::Float, &cooldown, 0.0f, 2.0f});
        fields.push_back({"Attack Range", InspectorField::FieldType::Float, &range, 0.0f, 3.0f});
    }

    AppEngine->GetEditorUIModule()->DrawScriptInspector(fields);
}

void Character::OnCollision(GameObject* otherObject, const float3& collisionNormal)
{

}

void Character::Attack(float deltaTime)
{
    if (CanAttack(deltaTime))
    {
        lastAttackTime = deltaTime;
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

bool Character::CanAttack(float deltaTime)
{
    deltaTime *= 1000.0f;
    if (deltaTime - lastAttackTime >= cooldown) return true;

    return false;
}

void Character::Die()
{
    GLOG("%s dead", parent->GetName().c_str());
    isDead = true;
    OnDeath();

    characterCollider->DeleteRigidBody();
    characterCollider->SetEnabled(false);

    weaponCollider->DeleteRigidBody();
    weaponCollider->SetEnabled(false);

    parent->SetEnabled(false);
}
