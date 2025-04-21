#include "pch.h"

#include "Application.h"
#include "EditorUIModule.h"
#include "ExitGameScript.h"
#include "GameObject.h"
#include "Scene/Components/Standalone/UI/ButtonComponent.h"
#include "Utils/Delegate.h"

bool ExitGameScript::Init()
{
    Component* button = parent->GetComponentByType(COMPONENT_BUTTON);
    if (button)
    {
        std::function<void(void)> function = std::bind(&ExitGameScript::OnClick, this);
        Delegate<void> delegate(function);
        delegateID            = static_cast<ButtonComponent*>(button)->AddOnClickCallback(delegate);
        hasRegisteredCallback = true;
    }

    return true;
}

void ExitGameScript::Update(float deltaTime)
{
}

void ExitGameScript::Inspector()
{
}

void ExitGameScript::OnClick()
{
    GLOG("Exiting game...");
    AppEngine->GetEditorUIModule()->RequestExit();
}

ExitGameScript::~ExitGameScript()
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
