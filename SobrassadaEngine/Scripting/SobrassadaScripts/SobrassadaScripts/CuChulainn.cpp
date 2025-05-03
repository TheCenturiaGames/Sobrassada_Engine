#include "pch.h"

#include "Application.h"
#include "Component.h"
#include "CuChulainn.h"
#include "GameObject.h"
#include "InputModule.h"
#include "ResourceStateMachine.h"
#include "Scene.h"
#include "SceneModule.h"
#include "Standalone/AnimationComponent.h"
#include "Standalone/CharacterControllerComponent.h"
#include "ScriptComponent.h"
#include "CameraMovement.h"
#include "CameraComponent.h"

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
    fields.push_back({"Camera Object", InspectorField::FieldType::InputText, &cameraName});
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

    return true;
}

void CuChulainn::Update(float deltaTime)
{
    if (isDead || !character) return;

    GetInputs();
    Character::Update(deltaTime);
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

void CuChulainn::PerformAttack()
{
    // TODO: play basicAttack sound
    // TODO: make interaction with hitboxes with the enemy ones
    // TODO: activate and disable the box collider located on one on the gameobjects bones
}

void CuChulainn::HandleState(float time)
{

    // Check dash
    bool canDash = true; // Maybe in the future you can't dash in some situations
    if (desiredDash && canDash)
    {
        desiredDash = false;
        state       = CharacterStates::DASH;
        if (animComponent) animComponent->UseTrigger("dash");
        // Dash
    }
    else if (desiredAttack && CanAttack(time))
    {
        GLOG("ATTACK");
        desiredAttack = false;
        state         = CharacterStates::BASIC_ATTACK;
        Attack(time);
        if (animComponent) animComponent->UseTrigger("attack");
        LookAtMouse();
        character->EnableMovement(false);
    }
    else if (desiredAim && CanAim())
    {
        if (state != CharacterStates::AIM)
        {
            camera->EnableMouseOffset(true);
            state = CharacterStates::AIM;
        }
        desiredAim = false;
        LookAtMouse();
        character->EnableMovement(false);
    }
    else if (!isAttacking && !isDashing)
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

    // When finished attacking, go back to idle state
    if (animComponent && animComponent->IsFinished())
    {
        state = CharacterStates::IDLE;
        animComponent->UseTrigger("idle");
    }

    // if (keyboard[SDL_SCANCODE_R] == KEY_REPEAT) stateMachine->UseTrigger("Basic_Attack");
    // else stateMachine->UseTrigger("Idle");
}

void CuChulainn::SetAnimations()
{
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

bool CuChulainn::CanAttack(float time)
{
    return (state != CharacterStates::DASH && !isAttacking && time - lastAttackTime >= attackCooldown);
}

bool CuChulainn::CanAim() const
{
    return (state != CharacterStates::DASH && state != CharacterStates::BASIC_ATTACK && throwTimer <= 0);
}

void CuChulainn::UpdateTimers(float deltaTime)
{
    if (desiredDash)
    {
        dashBufferTimer -= deltaTime;
        if (dashBufferTimer < 0) desiredDash = false;
    }

    if (desiredAttack)
    {
        attackBufferTimer -= deltaTime;
        if (attackBufferTimer < 0) desiredAttack = false;
    }

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
}