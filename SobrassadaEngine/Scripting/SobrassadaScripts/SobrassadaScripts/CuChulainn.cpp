#include "pch.h"

#include "Application.h"
#include "Component.h"
#include "CuChulainn.h"
#include "GameObject.h"
#include "ResourceStateMachine.h"
#include "Standalone/AnimationComponent.h"
#include "Standalone/CharacterControllerComponent.h"
#include "Modules/InputModule.h"

CharacterControllerComponent* character = nullptr;

CuChulainn::CuChulainn(GameObject* parent) : Character(parent, 5, 1, 2.0f, 1.0f, 1.0f)
{
}

bool CuChulainn::Init()
{
    GLOG("Initiating CuChulainn");

    Component* agent = parent->GetComponentByType(COMPONENT_CHARACTER_CONTROLLER);
    if (!agent)
    {
        GLOG("Component CharacterController not found for CuChulainn");
        return false;
    }

    character = dynamic_cast<CharacterControllerComponent*>(agent);

    stateMap  = {
        {"Idle",         CharacterStates::IDLE        },
        {"Run",          CharacterStates::RUN         },
        {"Dash",         CharacterStates::DASH        },
        {"Basic_Attack", CharacterStates::BASIC_ATTACK}
    };
    animComponent = parent->GetAnimationComponent();
    animComponent->OnPlay(false); // Starts On Idle

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

    ResourceStateMachine* stateMachine = animComponent->GetResourceStateMachine();
    if (!stateMachine) return;

    const KeyState* keyboard = AppEngine->GetInputModule()->GetKeyboard();
    const bool move          = keyboard[SDL_SCANCODE_W] == KEY_DOWN || keyboard[SDL_SCANCODE_D] == KEY_DOWN ||
                      keyboard[SDL_SCANCODE_A] == KEY_DOWN || keyboard[SDL_SCANCODE_S] == KEY_DOWN;
    
    if (move && !runActive)
    {
        stateMachine->UseTrigger("Run");
        runActive = true;
    }
    else if (runActive && !move)
    {
        stateMachine->UseTrigger("Idle");
        runActive = false;
    }


    // If(Input de dash){
    //stateMachine->UseTrigger("Dash");
    ////}else Deja de dashear{
    //stateMachine->UseTrigger("Idle");

    //if (keyboard[SDL_SCANCODE_R] == KEY_REPEAT) stateMachine->UseTrigger("Basic_Attack");
    //else stateMachine->UseTrigger("Idle");
}
