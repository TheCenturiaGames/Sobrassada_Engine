#include "pch.h"

#include "Application.h"
#include "Component.h"
#include "CuChulainn.h"
#include "GameObject.h"
#include "Modules/InputModule.h"
#include "ResourceStateMachine.h"
#include "Standalone/AnimationComponent.h"
#include "Standalone/CharacterControllerComponent.h"

CharacterControllerComponent* character = nullptr;

CuChulainn::CuChulainn(GameObject* parent) : Character(parent, 5, 1, 2.0f, 1.0f, 1.0f)
{
}

bool CuChulainn::Init()
{
    GLOG("Initiating CuChulainn");

    Character::Init();

    Component* agent = parent->GetComponentByType(COMPONENT_CHARACTER_CONTROLLER);
    if (!agent)
    {
        GLOG("CharacterController component not found for CuChulainn");
        return false;
    }

    character = dynamic_cast<CharacterControllerComponent*>(agent);

    return true;
}

void CuChulainn::Update(float deltaTime)
{
    HandleAnimation();
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

void CuChulainn::HandleAnimation()
{
    if (!animComponent) return;

    const KeyState* keyboard = AppEngine->GetInputModule()->GetKeyboard();
    const bool move          = keyboard[SDL_SCANCODE_W] || keyboard[SDL_SCANCODE_D] ||
                      keyboard[SDL_SCANCODE_A]|| keyboard[SDL_SCANCODE_S];

    GLOG("%d", move);
    if (move && !runActive)
    {
        triggerAvailable = animComponent->UseTrigger("Run");
        runActive        = true;
        
    }
    else if (runActive && !move)
    {
        triggerAvailable = animComponent->UseTrigger("idle");
        runActive = false;
    }

    


    // If(Input de dash){
    // stateMachine->UseTrigger("Dash");
    ////}else Deja de dashear{
    // stateMachine->UseTrigger("Idle");

    // if (keyboard[SDL_SCANCODE_R] == KEY_REPEAT) stateMachine->UseTrigger("Basic_Attack");
    // else stateMachine->UseTrigger("Idle");
}
