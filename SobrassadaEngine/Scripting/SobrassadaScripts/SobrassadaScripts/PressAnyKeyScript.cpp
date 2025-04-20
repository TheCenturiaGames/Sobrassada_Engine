#include "pch.h"

#include "PressAnyKeyScript.h"
#include "Application.h"
#include "GameObject.h"
#include "InputModule.h"
#include "Scene.h"
#include "SceneModule.h"
#include <imgui.h>

bool PressAnyKeyScript::Init()
{
    return true;
}

void PressAnyKeyScript::Update(float deltaTime)
{
    if (!parent->IsEnabled()) return;

    const KeyState* keys = AppEngine->GetInputModule()->GetKeyboard();

    for (int i = 0; i < SDL_NUM_SCANCODES; ++i)
    {
        if (keys[i] == KEY_DOWN)
        {
            parent->SetEnabled(false);

            const auto& gameObjects = AppEngine->GetSceneModule()->GetScene()->GetAllGameObjects();
            for (const auto& pair : gameObjects)
            {
                if (pair.second->GetName() == nextGameObjectName)
                {
                    pair.second->SetEnabled(true);
                    break;
                }
            }
            break;
        }
    }
}

void PressAnyKeyScript::Inspector()
{
    ImGui::SetCurrentContext(AppEngine->GetEditorUIModule()->GetImGuiContext());
    AppEngine->GetEditorUIModule()->DrawScriptInspector(
        [this]()
        {
            char buffer[128];
            strncpy_s(buffer, sizeof(buffer), panelToShowName.c_str(), _TRUNCATE);
            buffer[sizeof(buffer) - 1] = '\0';

            if (ImGui::InputText("Panel to Show", buffer, sizeof(buffer)))
            {
                panelToShowName = buffer;
            }
        }
    );
}

void PressAnyKeyScript::SaveToJson(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
{
    value.AddMember("NextGameObject", rapidjson::Value(nextGameObjectName.c_str(), allocator), allocator);
}

void PressAnyKeyScript::LoadFromJson(const rapidjson::Value& value)
{
    if (value.HasMember("NextGameObject") && value["NextGameObject"].IsString())
    {
        nextGameObjectName = value["NextGameObject"].GetString();
    }
}
