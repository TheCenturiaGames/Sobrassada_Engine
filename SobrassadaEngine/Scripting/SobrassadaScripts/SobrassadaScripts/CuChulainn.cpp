#include "pch.h"

#include "Component.h"
#include "CuChulainn.h"
#include "GameObject.h"
#include "Standalone/CharacterControllerComponent.h"
#include "ResourceStateMachine.h"
#include "Standalone/AnimationComponent.h"

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

    return true;
}

void CuChulainn::Update(float deltaTime)
{
    HandleAnimation();
}

void CuChulainn::OnDeath()
{
}

void CuChulainn::OnDamageTaken(int amount)
{
}

void CuChulainn::OnHealed(int amount)
{
}

void CuChulainn::PerformAttack()
{
}

void CuChulainn::HandleAnimation()
{
    AnimationComponent* animComponent = parent->GetAnimationComponent();
    if (!animComponent) return;

    ResourceStateMachine* stateMachine = animComponent->GetResourceStateMachine();
    if (!stateMachine) return;

    const State* activeState = stateMachine->GetActiveState();
    if (!activeState) return;

    std::string stateName        = activeState->name.GetString();
    CharacterStates currentState = CharacterStates::NONE;

    auto it                      = stateMap.find(stateName);
    if (it != stateMap.end()) currentState = it->second;

    switch (currentState)
    {
    case CharacterStates::IDLE:
        // TODO: Play IDLE ANIMATION
        break;
    case CharacterStates::RUN:
        // TODO: Play RUN ANIMATION
        break;
    case CharacterStates::DASH:
        // TODO: Play DASH ANIMATION
        break;
    case CharacterStates::BASIC_ATTACK:
        // TODO: Play BASIC_ATTACK ANIMATION
        break;
    }
}
