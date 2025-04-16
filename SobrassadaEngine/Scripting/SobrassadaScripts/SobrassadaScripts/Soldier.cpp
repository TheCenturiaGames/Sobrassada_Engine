#include "pch.h"

#include "Component.h"
#include "CuChulainn.h"
#include "GameObject.h"
#include "Soldier.h"
#include "Standalone/AIAgentComponent.h"
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

    agentAI = dynamic_cast<AIAgentComponent*>(agent);

    return true;
}

void Soldier::Update(float deltaTime)
{
    if (character != nullptr)
    {
        agentAI->SetPathNavigation(character->GetLastPosition());
    }
        
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
