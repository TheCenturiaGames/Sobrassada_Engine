#include "pch.h"

#include "VSyncToggleScript.h"
#include "Application.h"
#include "EditorUIModule.h"
#include "GameObject.h"
#include "Scene/Components/Standalone/UI/ButtonComponent.h"
#include "Utils/Delegate.h"

bool VSyncToggleScript::Init()
{
    Component* button = parent->GetComponentByType(COMPONENT_BUTTON);
    if (button)
    {
        std::function<void(void)> function = std::bind(&VSyncToggleScript::OnClick, this);
        Delegate<void> delegate(function);
        delegateID            = static_cast<ButtonComponent*>(button)->AddOnClickCallback(delegate);
        hasRegisteredCallback = true;
    }

    return true;
}

void VSyncToggleScript::Update(float deltaTime)
{
}

void VSyncToggleScript::Inspector()
{
}

void VSyncToggleScript::OnClick()
{
    AppEngine->GetEditorUIModule()->ToggleVSync();
}

VSyncToggleScript::~VSyncToggleScript()
{
    if (hasRegisteredCallback)
    {
        Component* button = parent->GetComponentByType(COMPONENT_BUTTON);
        if (button)
        {
            static_cast<ButtonComponent*>(button)->RemoveOnClickCallback(delegateID);
        }
    }
}

void VSyncToggleScript::OnDestroy()
{
    hasRegisteredCallback = false;
    delegateID            = {};
}
