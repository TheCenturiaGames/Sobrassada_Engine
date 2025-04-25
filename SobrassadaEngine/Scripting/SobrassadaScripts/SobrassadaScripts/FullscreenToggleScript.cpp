#include "pch.h"

#include "Application.h"
#include "EditorUIModule.h"
#include "FullscreenToggleScript.h"
#include "GameObject.h"
#include "Standalone/UI/ButtonComponent.h"

bool FullscreenToggleScript::Init()
{
    // Component* button = parent->GetComponentByType(COMPONENT_BUTTON);

    // if (button)
    //{
    //     FullscreenToggleScript* self   = this;

    //    std::function<void()> function = [self]()
    //    {
    //        if (self) self->OnClick();
    //    };

    //    Delegate<void> delegate(function);
    //    delegateID            = static_cast<ButtonComponent*>(button)->AddOnClickCallback(delegate);
    //    hasRegisteredCallback = true;
    //}

    ButtonComponent* button = parent->GetComponent<ButtonComponent*>();
    if (button)
    {
        FullscreenToggleScript* self   = this;

        std::function<void()> function = [self]()
        {
            if (self) self->OnClick();
        };

        Delegate<void> delegate(function);
        delegateID            = button->AddOnClickCallback(delegate);
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
        /*Component* button = parent->GetComponentByType(COMPONENT_BUTTON);
        if (button)
        {
            static_cast<ButtonComponent*>(button)->RemoveOnClickCallback(delegateID);
        }*/

        ButtonComponent* button = parent->GetComponent<ButtonComponent*>();
        if (button) button->RemoveOnClickCallback(delegateID);
    }
}

void FullscreenToggleScript::OnDestroy()
{
    hasRegisteredCallback = false;
    delegateID            = {};
}
