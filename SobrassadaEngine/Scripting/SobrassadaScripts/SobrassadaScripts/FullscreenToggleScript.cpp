#include "pch.h"

#include "FullscreenToggleScript.h"
#include "Application.h"
#include "EditorUIModule.h"
#include "GameObject.h"
#include "Scene/Components/Standalone/UI/ButtonComponent.h"
#include "Utils/Delegate.h"

bool FullscreenToggleScript::Init()
{
    Component* button = parent->GetComponentByType(COMPONENT_BUTTON);

    if (button)
    {
        std::function<void(void)> function = std::bind(&FullscreenToggleScript::OnClick, this);
        Delegate<void> delegate(function);
        delegateID            = static_cast<ButtonComponent*>(button)->AddOnClickCallback(delegate);
        hasRegisteredCallback = true;
    }

    return true;
}

void FullscreenToggleScript::Update(float deltaTime)
{
}

void FullscreenToggleScript::Inspector()
{
}

void FullscreenToggleScript::OnClick()
{
    AppEngine->GetEditorUIModule()->ToggleFullscreen();
}

FullscreenToggleScript::~FullscreenToggleScript()
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
