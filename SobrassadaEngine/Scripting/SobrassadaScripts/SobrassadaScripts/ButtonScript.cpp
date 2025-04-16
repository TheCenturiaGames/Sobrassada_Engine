#include "pch.h"
#include "ButtonScript.h"
#include "GameObject.h"
#include "Component.h"
#include "Scene/Components/Standalone/UI/ButtonComponent.h"
#include "Utils/Delegate.h"

bool ButtonScript::Init()
{
    GLOG("Initiating ButtonScript");

    Component* button = parent->GetComponentByType(COMPONENT_BUTTON);
    if (button)
    {
        std::function<void(void)> function = std::bind(&ButtonScript::TestDispatcher, this);
        Delegate<void> delegate(function);
        static_cast<ButtonComponent*>(button)->AddOnClickCallback(delegate);
    }     

    return true;
}

void ButtonScript::Update(float deltaTime)
{
}

void ButtonScript::TestDispatcher()
{
    GLOG("Execute function!");
}
