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

    Component* agent = parent->GetComponentByType(COMPONENT_AIAGENT);
    if (!agent)
    {
        GLOG("Component AIAgent not found for Soldier");
        return false;
    }

    agentAI  = dynamic_cast<AIAgentComponent*>(agent);

    stateMap = {
        {"Patrol",       SoldierStates::PATROL      },
        {"Chase",        SoldierStates::CHASE       },
        {"Basic_Attack", SoldierStates::BASIC_ATTACK}
    };
    animComponent = parent->GetAnimationComponent();
    animComponent->OnPlay(false); // Starts On Patrol

    return true;
}

void Soldier::Update(float deltaTime)
{
    HandleState(deltaTime);
}

void Soldier::OnDeath()
{
}

void Soldier::OnDamageTaken(int amount)
{
}

void Soldier::OnHealed(int amount)
{
}

void Soldier::PerformAttack()
{
}

void Soldier::HandleState(float deltaTime)
{
    if (!animComponent) return;

    ResourceStateMachine* stateMachine = animComponent->GetResourceStateMachine();
    if (!stateMachine) return;

    const State* activeState = stateMachine->GetActiveState();
    if (!activeState) return;

    std::string stateName      = activeState->name.GetString();
    SoldierStates currentState = SoldierStates::NONE;

    auto it                    = stateMap.find(stateName);
    if (it != stateMap.end()) currentState = it->second;

    switch (currentState)
    {
    case SoldierStates::PATROL:
        GLOG("Soldier Patrolling");
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
        break;
    }
}

void Soldier::ChaseAI()
{
    if (character != nullptr)
    {
        agentAI->SetPathNavigation(character->GetLastPosition());
    }
}
