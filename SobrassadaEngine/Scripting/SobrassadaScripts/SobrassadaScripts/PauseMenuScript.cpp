#include "pch.h"
#include "PauseMenuScript.h"

#include "Application.h"
#include "GameObject.h"
#include "InputModule.h"
#include "Scene.h"
#include "SceneModule.h"
#include "GameObject.h"

bool PauseMenuScript::Init()
{
    return true;
}

void PauseMenuScript::Update(float deltaTime)
{
    const KeyState* keys = AppEngine->GetInputModule()->GetKeyboard();

    if (keys[SDL_SCANCODE_ESCAPE] == KEY_DOWN)
    {
        const auto& gameObjects = AppEngine->GetSceneModule()->GetScene()->GetAllGameObjects();
        for (const auto& pair : gameObjects)
        {
            GameObject* go = pair.second;
            if (go->GetName() == panelToShowName)
            {
                go->SetEnabled(!go->IsEnabled());
                break;
            }
        }
    }
}
