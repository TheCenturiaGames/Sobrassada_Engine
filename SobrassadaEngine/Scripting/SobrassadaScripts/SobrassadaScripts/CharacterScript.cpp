#include "pch.h"
#include "CharacterScript.h"
#include "Component.h"
#include "Scene/Components/Standalone/AnimationComponent.h"
#include "ResourceStateMachine.h"
#include "GameObject.h"


bool CharacterScript::Init()
{
    GLOG("Initiating CharacterScript");
    return true;
}

void CharacterScript::Update(float deltaTime)
{
   // AnimationComponent* animComponent = parent
}