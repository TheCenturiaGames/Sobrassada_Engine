#include "pch.h"
#include "FullscreenToggleScript.h"

#include "Application.h"
#include "Scene/Components/Standalone/UI/ButtonComponent.h"
#include "GameObject.h"
#include "Utils/Delegate.h"
#include "EditorUIModule.h"




bool FullscreenToggleScript::Init()
{
    Component* button = parent->GetComponentByType(COMPONENT_BUTTON);

    if (button)
    {
        std::function<void(void)> function = std::bind(&FullscreenToggleScript::OnClick, this);
        Delegate<void> delegate(function);
        static_cast<ButtonComponent*>(button)->AddOnClickCallback(delegate);
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
