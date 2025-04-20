#include "pch.h"

#include "Application.h"
#include "GameObject.h"
#include "InputModule.h"
#include "OptionsMenuSwitcherScript.h"
#include "Scene.h"
#include "SceneModule.h"

bool OptionsMenuSwitcherScript::Init()
{
    ShowOnlyCurrentPanel();
    return true;
}

void OptionsMenuSwitcherScript::Update(float deltaTime)
{

    if (!parent->IsEnabled()) return;

    if (!initialized)
    {
        ShowOnlyCurrentPanel();
        initialized = true;
    }

    const KeyState* keys = AppEngine->GetInputModule()->GetKeyboard();

    if (keys[SDL_SCANCODE_Q] == KEY_DOWN || keys[SDL_SCANCODE_E] == KEY_DOWN)
    {
        // Deactivate current
        GameObject* currentGO = FindPanelByName(panelNames[currentIndex]);
        if (currentGO) currentGO->SetEnabled(false);

        // Update index
        if (keys[SDL_SCANCODE_Q] == KEY_DOWN) currentIndex = (currentIndex - 1 + panelNames.size()) % panelNames.size();
        else if (keys[SDL_SCANCODE_E] == KEY_DOWN) currentIndex = (currentIndex + 1) % panelNames.size();

        // Activate new
        ShowOnlyCurrentPanel();
    }

    if (keys[SDL_SCANCODE_ESCAPE] == KEY_DOWN)
    {
        // Disable all panels in the options menu
        for (const std::string& name : panelNames)
        {
            GameObject* panel = FindPanelByName(name);
            if (panel) panel->SetEnabled(false);
        }

        // Enable the MainMenuPanel
        GameObject* mainMenuPanel = FindPanelByName("MainMenuPanel");
        if (mainMenuPanel) mainMenuPanel->SetEnabled(true);

        parent->SetEnabled(false);
        initialized = false;
    }
}

void OptionsMenuSwitcherScript::Inspector()
{
}

void OptionsMenuSwitcherScript::ShowOnlyCurrentPanel()
{
    for (int i = 0; i < panelNames.size(); ++i)
    {
        GameObject* panel = FindPanelByName(panelNames[i]);
        if (panel) panel->SetEnabled(i == currentIndex);
    }
}

GameObject* OptionsMenuSwitcherScript::FindPanelByName(const std::string& name) const
{
    const auto& gameObjects = AppEngine->GetSceneModule()->GetScene()->GetAllGameObjects();
    for (const auto& pair : gameObjects)
    {
        if (pair.second->GetName() == name) return pair.second;
    }
    return nullptr;
}

void OptionsMenuSwitcherScript::SaveToJson(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
{
    value.AddMember("CurrentPanelIndex", currentIndex, allocator);
}

void OptionsMenuSwitcherScript::LoadFromJson(const rapidjson::Value& value)
{
    if (value.HasMember("CurrentPanelIndex") && value["CurrentPanelIndex"].IsInt())
    {
        currentIndex = value["CurrentPanelIndex"].GetInt();
    }
}
