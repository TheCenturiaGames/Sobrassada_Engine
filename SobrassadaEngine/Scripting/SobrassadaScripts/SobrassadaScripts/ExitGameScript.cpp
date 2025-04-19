#include "pch.h"

#include "ExitGameScript.h"
#include "Application.h"
#include "GameObject.h"
#include "EditorUIModule.h"
#include "Scene/Components/Standalone/UI/ButtonComponent.h"
#include "Utils/Delegate.h"
#include <imgui.h>


bool ExitGameScript::Init()
{
    Component* button = parent->GetComponentByType(COMPONENT_BUTTON);
    if (button)
    {
        std::function<void(void)> function = std::bind(&ExitGameScript::OnClick, this);
        Delegate<void> delegate(function);
        static_cast<ButtonComponent*>(button)->AddOnClickCallback(delegate);
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
