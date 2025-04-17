#include "pch.h"
#include "ButtonScript.h"
#include "GameObject.h"
#include "Component.h"
#include "Application.h"
#include "GameUIModule.h"
#include "Scene/Components/Standalone/UI/ButtonComponent.h"
#include "Utils/Delegate.h"
#include "SceneModule.h"


bool ButtonScript::Init()
{
    GLOG("Initiating ButtonScript");

    Component* button = parent->GetComponentByType(COMPONENT_BUTTON);
    if (button)
    {
        std::function<void(void)> function = std::bind(&ButtonScript::OnClick, this);
        Delegate<void> delegate(function);
        static_cast<ButtonComponent*>(button)->AddOnClickCallback(delegate);
    }     

    return true;
}

void ButtonScript::Update(float /*deltaTime*/)
{
}

void ButtonScript::OnClick()
{
    Scene* scene            = AppEngine->GetSceneModule()->GetScene();
    const auto& gameObjects = scene->GetAllGameObjects();

    for (const auto& [uid, go] : gameObjects)
    {
        if (go && go->GetName() == "MainMenuPanel")
        {
            go->SetEnabled(false);
        }

        if (go && go->GetName() == "OptionsPanel")
        {
            go->SetEnabled(true);
        }
    }
}
