#include "pch.h"
#include "UIButtonScript.h"
#include "Application.h"
#include "GameObject.h"
#include "GameUIModule.h"
#include <Standalone/UI/ButtonComponent.h>

bool UIButtonScript::Init()
{
    Component* component    = parent->GetComponentByType(COMPONENT_BUTTON);
    ButtonComponent* button = static_cast<ButtonComponent*>(component);

    if (button)
    {
        button->AddOnClickCallback(Delegate<void>::FromFunction([=]() { this->OnButtonClicked(); }));
    }
    else
    {
        GLOG("[UIButtonScript] Warning: No ButtonComponent found on GameObject '%s'", parent->GetName().c_str());
    }

    return true;
}

void UIButtonScript::Update(float /*deltaTime*/)
{
}

void UIButtonScript::OnButtonClicked()
{
   // App->GetGameUIModule()->SwitchToScreen(screenToActivate);
    GLOG("[UIButtonScript] Switched to screen: %s", screenToActivate.c_str());
}
