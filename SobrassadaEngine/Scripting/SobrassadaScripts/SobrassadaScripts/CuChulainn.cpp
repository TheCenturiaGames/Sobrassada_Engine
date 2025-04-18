#include "pch.h"

#include "Component.h"
#include "CuChulainn.h"
#include "GameObject.h"
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
    if (!animComponent) return;

    ResourceStateMachine* stateMachine = animComponent->GetResourceStateMachine();
    if (!stateMachine) return;

    // If(input de correr){
    stateMachine->UseTrigger("Run");
    //}else Deja de correr{
    stateMachine->UseTrigger("Idle");

    // If(Input de dash){
    stateMachine->UseTrigger("Dash");
    //}else Deja de dashear{
    stateMachine->UseTrigger("Idle");

    // If(Input de ataque){
    stateMachine->UseTrigger("Basic_Attack");
    //}else Deja de atacar{
    stateMachine->UseTrigger("Idle");
}
