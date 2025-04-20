#include "pch.h"

#include "PressAnyKeyScript.h"
#include "Application.h"
#include "GameObject.h"
#include "InputModule.h"
#include "Scene.h"
#include "SceneModule.h"

bool PressAnyKeyScript::Init()
{
    return true;
}

void PressAnyKeyScript::Update(float deltaTime)
{
    const KeyState* keys = AppEngine->GetInputModule()->GetKeyboard();

    if (!inputReady)
    {
        // Wait untill all keys are IDLE
        bool allIdle = true;
        for (int i = 0; i < SDL_NUM_SCANCODES; ++i)
        {
            if (keys[i] != KEY_IDLE)
            {
                allIdle = false;
                break;
            }
        }

        if (allIdle)
        {
            inputReady = true; 
        }

        return; 
    }

    // Any key
    for (int i = 0; i < SDL_NUM_SCANCODES; ++i)
    {
        if (keys[i] == KEY_DOWN)
        {
            // Disable current GO
            parent->SetEnabled(false);

            // Enable MainMenuPanel
            Scene* scene            = AppEngine->GetSceneModule()->GetScene();
            const auto& gameObjects = scene->GetAllGameObjects();
            for (const auto& pair : gameObjects)
            {
                if (pair.second->GetName() == "MainMenuPanel")
                {
                    pair.second->SetEnabled(true);
                    break;
                }
            }
            break;
        }
    }
}
