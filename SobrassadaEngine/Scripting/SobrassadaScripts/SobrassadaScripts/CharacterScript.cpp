#include "pch.h"
#include "CharacterScript.h"
#include "Component.h"
#include "Scene/Components/Standalone/AnimationComponent.h"
#include "ResourceStateMachine.h"
#include "GameObject.h"


CharacterScript::CharacterScript(GameObject* parent) : Script(parent)
{
    maxHealth = 10;
    currentHealth = maxHealth;
    damage        = 1;
    speed         = 2.0f;
    cooldown      = 100.0f;
    lastAttackTime = 500.0f;
    range          = 2.0f;
}

bool CharacterScript::Init()
{
    GLOG("Initiating CharacterScript");
    
    stateMap = {
        {"Idle", CharacterStates::IDLE},
        {"Run",  CharacterStates::RUN },
        {"Dash", CharacterStates::DASH},
        {"Basic_Attack", CharacterStates::BASIC_ATTACK}
    };

    return true;
}

void CharacterScript::Update(float deltaTime)
{
    HandleAnimation();
}

void CharacterScript::Kill()
{
    GLOG("Killed Character");
    isDead = true;
}

void CharacterScript::TakeDamage(int amount)
{
    currentHealth -= amount;

    if (currentHealth <= 0) Kill();
}

void CharacterScript::Heal(int amount)
{
    currentHealth += amount;

    if (currentHealth > maxHealth) currentHealth = maxHealth;
}

void CharacterScript::HandleAnimation()
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
