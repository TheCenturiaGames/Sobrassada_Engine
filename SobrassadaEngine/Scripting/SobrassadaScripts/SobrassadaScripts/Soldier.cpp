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

Soldier::Soldier(GameObject* parent) : Character(parent, 3, 1, 0.5f, 2.0f, 1.0f, 1.0f)
{
}

bool Soldier::Init()
{
    GLOG("Initiating Soldier");

    currentState = SoldierStates::CHASE;

    Character::Init();

    Component* agent = parent->GetComponentByType(COMPONENT_AIAGENT);
    if (!agent)
    {
        GLOG("AIAgent component not found for Soldier");
        return false;
    }

    agentAI = dynamic_cast<AIAgentComponent*>(agent);
    agentAI->SetSpeed(speed);

    return true;
}

void Soldier::Update(float deltaTime)
{
    Character::Update(deltaTime);
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

    switch (currentState)
    {
    case SoldierStates::PATROL:
        // GLOG("Soldier Patrolling");
        animComponent->UseTrigger("walk");
        PatrolAI();
        break;
    case SoldierStates::CHASE:
        // GLOG("Soldier Chasing");
        animComponent->UseTrigger("Run");
        ChaseAI();
        break;
    case SoldierStates::BASIC_ATTACK:
        // GLOG("Soldier Basic Attack");
        animComponent->UseTrigger("Idle");
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
        if (!agentAI->SetPathNavigation(character->GetLastPosition())) currentState = SoldierStates::PATROL;
    }
    else currentState = SoldierStates::PATROL;
}
