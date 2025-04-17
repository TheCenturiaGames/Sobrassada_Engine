#include "pch.h"
#include "GodMode.h"
#include "Application.h"
#include "CameraComponent.h"
#include "Components/Standalone/CharacterControllerComponent.h"
#include "GameObject.h"
#include "InputModule.h"
#include <SDL_mouse.h>

bool GodMode::Init()
{
    characterController =
        dynamic_cast<CharacterControllerComponent*>(parent->GetComponentByType(COMPONENT_CHARACTER_CONTROLLER));
    godCamera = dynamic_cast<CameraComponent*>(parent->GetComponentChildByType(COMPONENT_CAMERA));

    return true;
}

void GodMode::Update(float deltaTime)
{
    const KeyState* keyboard = AppEngine->GetInputModule()->GetKeyboard();

    if (freeCamera)
    {
        const KeyState* mouseButtons = AppEngine->GetInputModule()->GetMouseButtons();
        const float2& mouseMotion    = AppEngine->GetInputModule()->GetMouseMotion();

        float scaleFactor            = 1.0f;
        if (keyboard[SDL_SCANCODE_LSHIFT]) scaleFactor *= 2;

        float finalCameraSpeed       = 7.5f * scaleFactor * deltaTime;
        float finalRotateSensitivity = 0.006f * scaleFactor;

        if (keyboard[SDL_SCANCODE_W]) godCamera->Translate(godCamera->GetCameraFront() * finalCameraSpeed);
        if (keyboard[SDL_SCANCODE_S]) godCamera->Translate(-godCamera->GetCameraFront() * finalCameraSpeed);
        if (keyboard[SDL_SCANCODE_A]) godCamera->Translate(-godCamera->GetCameraRight() * finalCameraSpeed);
        if (keyboard[SDL_SCANCODE_D]) godCamera->Translate(godCamera->GetCameraRight() * finalCameraSpeed);
        if (keyboard[SDL_SCANCODE_E]) godCamera->Translate(godCamera->GetCameraUp() * finalCameraSpeed);
        if (keyboard[SDL_SCANCODE_Q]) godCamera->Translate(-godCamera->GetCameraUp() * finalCameraSpeed);

        if (mouseButtons[SDL_BUTTON_RIGHT - 1])
        {
            float mouseX             = mouseMotion.x;
            float mouseY             = mouseMotion.y;
            float deltaRotationAngle = cameraRotationAngle * finalRotateSensitivity;
            godCamera->Rotate(-mouseX * deltaRotationAngle, -mouseY * deltaRotationAngle);
        }

        if (keyboard[SDL_SCANCODE_O] == KEY_DOWN)
        {
            freeCamera = false;
            godCamera->SetFreeCamera(false);
            characterController->SetInputDown(true);
        }
    }
    if (keyboard[SDL_SCANCODE_P] == KEY_DOWN)
    {
        characterController->SetInputDown(false);
        godCamera->SetFreeCamera(true);
        freeCamera = true;
    }
}

void GodMode::Inspector()
{
}