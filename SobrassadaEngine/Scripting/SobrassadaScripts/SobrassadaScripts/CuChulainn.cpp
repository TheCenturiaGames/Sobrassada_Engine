#include "pch.h"

#include "Application.h"
#include "CameraComponent.h"
#include "CameraMovement.h"
#include "Component.h"
#include "CuChulainn.h"
#include "GameObject.h"
#include "InputModule.h"
#include "Projectile.h"
#include "ResourceStateMachine.h"
#include "Scene.h"
#include "SceneModule.h"
#include "ScriptComponent.h"
#include "Standalone/AnimationComponent.h"
#include "Standalone/CharacterControllerComponent.h"

#include "SDL.h"

CharacterControllerComponent* character = nullptr;

CuChulainn::CuChulainn(GameObject* parent) : Character(parent, 5, 1, 0.5f, 2.0f, 1.0f, 1.0f, 0.0f, 0.0f)
{
    currentHealth = 3; // mainChar starts low hp

    fields.push_back({"Max Health", InspectorField::FieldType::Int, &maxHealth, 0, 10});
    fields.push_back({"Current Health", InspectorField::FieldType::Int, &currentHealth, 0, 10});
    fields.push_back({"Invulnerable", InspectorField::FieldType::Bool, &isInvulnerable, true, false});
    fields.push_back({"Dead", InspectorField::FieldType::Bool, &isDead, true, false});
    fields.push_back({"Damage", InspectorField::FieldType::Int, &damage, 0, 3});
    fields.push_back({"Attack Duration", InspectorField::FieldType::Float, &attackDuration, 0.0f, 1.0f});
    fields.push_back({"Attack Cooldown", InspectorField::FieldType::Float, &attackCooldown, 0.0f, 2.0f});
    fields.push_back({"Attack Range", InspectorField::FieldType::Float, &range, 0.0f, 3.0f});

    // TODO: Replace target names by gameObjects when overriding prefabs doesn't break the link
    fields.push_back({"Camera Object Name", InspectorField::FieldType::InputText, &cameraName});
    fields.push_back({"Spear Object Name", InspectorField::FieldType::InputText, &spearName});
}

bool CuChulainn::Init()
{
    GLOG("Initiating CuChulainn");

    Character::Init();

    character = parent->GetComponent<CharacterControllerComponent*>();
    if (!character)
    {
        GLOG("CharacterController component not found for CuChulainn");
        return false;
    }

    const GameObject* cameraObj = AppEngine->GetSceneModule()->GetScene()->GetGameObjectByName(cameraName);
    if (cameraObj && cameraObj->GetComponent<ScriptComponent*>())
    {
        camera = cameraObj->GetComponent<ScriptComponent*>()->GetScriptByType<CameraMovement>();
        if (!camera) GLOG("[WARNING] No camera found by the name %s", cameraName.c_str());
    }

    const GameObject* spearObj = AppEngine->GetSceneModule()->GetScene()->GetGameObjectByName(spearName);
    if (spearObj && spearObj->GetComponent<ScriptComponent*>())
    {
        spear = spearObj->GetComponent<ScriptComponent*>()->GetScriptByType<Projectile>();
        if (!spear) GLOG("[WARNING] No projectile found by the name %s", spearName.c_str());
    }

    return true;
}

void CuChulainn::Update(float deltaTime)
{
    if (isDead || !character) return;

    GetInputs();
    Character::Update(deltaTime);
    PerformAttack();
    UpdateTimers(deltaTime);
}

void CuChulainn::OnDeath()
{
    // TODO: include death sound for the character
}

void CuChulainn::OnDamageTaken(int amount)
{
    // TODO: play CuChulainn take damage sound
    // TODO: fill riastrad bar dinamically
}

void CuChulainn::OnHealed(int amount)
{
    // TODO: play CuChulainn recover sound
    // TODO: play particle system effects
}

void CuChulainn::HandleState(float time)
{
    if (desiredDash && CanDash()) Dash();
    else if (desiredAttack && CanAttack(time)) Attack(time);
    else if (desiredAim && CanAim()) Aim();
    else if (!isAttacking && !isDashing) Move();

    // When finished animation, go back to idle state
    if (animComponent && animComponent->IsFinished())
    {
        state = CharacterStates::IDLE;
        animComponent->UseTrigger("idle");
    }
}

void CuChulainn::GetInputs()
{
    const KeyState* keyboard = AppEngine->GetInputModule()->GetKeyboard();
    const KeyState* mouse    = AppEngine->GetInputModule()->GetMouseButtons();

    if (keyboard[SDL_SCANCODE_SPACE] == KEY_DOWN)
    {
        desiredDash     = true;
        dashBufferTimer = dashBuffer;
    }
    if (mouse[SDL_BUTTON_LEFT - 1] == KEY_DOWN)
    {
        desiredAttack     = true;
        attackBufferTimer = attackBuffer;
    }
    if (mouse[SDL_BUTTON_RIGHT - 1] == KEY_REPEAT)
    {
        desiredAim = true;
    }
    if (mouse[SDL_BUTTON_RIGHT - 1] == KEY_UP)
    {
        if (state == CharacterStates::AIM) ThrowSpear();
    }
}

bool CuChulainn::CanDash()
{
    // TODO: Add more condifions if there are (Maybe dashing doesn't cancel attack animations, etc.)
    return dashTimer <= 0;
}

bool CuChulainn::CanAttack(float time)
{
    return (state != CharacterStates::DASH && !isAttacking && time - lastAttackTime >= attackCooldown);
}

bool CuChulainn::CanAim() const
{
    return (state != CharacterStates::DASH && !isAttacking && throwTimer <= 0);
}

void CuChulainn::UpdateTimers(float deltaTime)
{
    // Dash timers
    dashTimer -= deltaTime;
    if (dashTimer < 0) dashTimer = 0;
    if (desiredDash)
    {
        dashBufferTimer -= deltaTime;
        if (dashBufferTimer < 0) desiredDash = false;
    }

    // Melee attack timers

    if (desiredAttack)
    {
        attackBufferTimer -= deltaTime;
        if (attackBufferTimer < 0) desiredAttack = false;
    }

    // Ranged attack timers
    desiredAim  = false;
    throwTimer -= deltaTime;
    if (throwTimer < 0) throwTimer = 0;
}

void CuChulainn::LookAtMouse()
{
    const float3 mouseWorldPos = AppEngine->GetSceneModule()->GetScene()->GetMainCamera()->ScreenPointToXZ(
        parent->GetGlobalTransform().TranslatePart().y
    );
    float3 direction = mouseWorldPos - parent->GetPosition();
    direction.y      = 0;
    direction.Normalize();
    character->LookAt(direction);
}

void CuChulainn::ThrowSpear()
{
    camera->EnableMouseOffset(false);
    GLOG("THROW SPEAR");
    throwTimer = throwCooldown;

    spear->Shoot(parent->GetPosition(), character->GetFrontDirection());
}

void CuChulainn::Dash()
{
    if (state == CharacterStates::AIM) camera->EnableMouseOffset(false);
    desiredDash = false;
    state       = CharacterStates::DASH;

    // TODO: Dash
    // character->Dash(direction)
    if (animComponent) animComponent->UseTrigger("dash");
}

void CuChulainn::PerformAttack()
{
    // TODO: make interaction with hitboxes with the enemy ones
    // TODO: activate and disable the box collider located on one on the gameobjects bones

    if (!isAttacking) return;

    // TODO: When timer matches animation, enable weapon collider. Disable it afterwards
}

void CuChulainn::Attack(float time)
{
    // TODO: play basicAttack sound

    GLOG("ATTACK");
    
    if (state == CharacterStates::AIM) camera->EnableMouseOffset(false);
    desiredAttack = false;
    state         = CharacterStates::BASIC_ATTACK;
    character->EnableMovement(false);
    
    Character::Attack(time);
    LookAtMouse();
    if (animComponent) animComponent->UseTrigger("attack");
}

void CuChulainn::Aim()
{
    if (state != CharacterStates::AIM)
    {
        camera->EnableMouseOffset(true);
        state = CharacterStates::AIM;
        character->EnableMovement(false);
    }
    desiredAim = false;

    LookAtMouse();
    if (animComponent) animComponent->UseTrigger("aim");
}

void CuChulainn::Move()
{
    character->EnableMovement(true);
    if (character->GetSpeed() > 0.5f)
    {
        if (state != CharacterStates::RUN && animComponent) animComponent->UseTrigger("run");
        state = CharacterStates::RUN;
    }
    else
    {
        if (state != CharacterStates::IDLE && animComponent) animComponent->UseTrigger("idle");
        state = CharacterStates::IDLE;
    }
}