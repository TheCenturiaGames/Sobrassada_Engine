#include "pch.h"

#include "Component.h"
#include "CuChulainn.h"
#include "GameObject.h"
#include "Globals.h"
#include "ResourceStateMachine.h"
#include "Soldier.h"
#include "Standalone/AIAgentComponent.h"
#include "Standalone/AnimationComponent.h"
#include "Standalone/CharacterControllerComponent.h"

Soldier::Soldier(GameObject* parent) : Character(parent, 5, 1, 2.0f, 1.0f, 1.0f)
{
}

bool Soldier::Init()
{
    GLOG("Initiating Soldier");

    Character::Init();

    Component* agent = parent->GetComponentByType(COMPONENT_AIAGENT);
    if (!agent)
    {
        GLOG("AIAgent component not found for Soldier");
        return false;
    }

    agentAI       = dynamic_cast<AIAgentComponent*>(agent);

    currentState = SoldierStates::PATROL;

    return true;
}

void Soldier::Update(float deltaTime)
{
    HandleState(deltaTime);
}

void Soldier::OnDeath()
{
    // TODO: include death sound for the character
    // TODO: animation and particles
}

void Soldier::OnDamageTaken(int amount)
{
    // TODO: play soldier take damage sound
    // TODO: particles? and animation
}

void Soldier::PerformAttack()
{
    // TODO: play basicAttack sound
    // TODO: make interaction with hitboxes with the character
    // TODO: activate and disable the box collider located on one on the gameobjects weapon
    // TODO: trails, particles and animation
}

void Soldier::HandleState(float deltaTime)
{
    if (!animComponent) return;

    ResourceStateMachine* stateMachine = animComponent->GetResourceStateMachine();
    if (!stateMachine) return;

    switch (currentState)
    {
    case SoldierStates::PATROL:
        GLOG("Soldier Patrolling");
        PatrolAI();
        break;
    case SoldierStates::CHASE:
        GLOG("Soldier Chasing");
        ChaseAI();
        break;
    case SoldierStates::BASIC_ATTACK:
        GLOG("Soldier Basic Attack");
        Attack(deltaTime);
        break;
    default:
        GLOG("No state provided to Soldier");
        currentState = SoldierStates::PATROL;
        break;
    }
}

void Soldier::PatrolAI()
{
}

void Soldier::ChaseAI()
{
    if (character != nullptr)
    {
        agentAI->SetPathNavigation(character->GetLastPosition());
    }
    else currentState = SoldierStates::PATROL;
}
