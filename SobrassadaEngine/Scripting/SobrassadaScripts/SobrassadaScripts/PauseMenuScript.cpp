#include "pch.h"

#include "PauseMenuScript.h"
#include "Application.h"
#include "GameObject.h"
#include "InputModule.h"
#include "Scene.h"
#include "SceneModule.h"

bool PauseMenuScript::Init()
{
    return true;
}

void PauseMenuScript::Update(float deltaTime)
{
    const KeyState* keys = AppEngine->GetInputModule()->GetKeyboard();

    if (keys[SDL_SCANCODE_ESCAPE] == KEY_DOWN)
    {
        const std::unordered_map<UID, GameObject*>& allGameObjects =
            AppEngine->GetSceneModule()->GetScene()->GetAllGameObjects();

        for (const std::pair<const UID, GameObject*>& gameObjectPair : allGameObjects)
        {
            GameObject* gameObject = gameObjectPair.second;

            if (gameObject->GetName() == panelToShowName)
            {
                UID parentUID        = gameObject->GetParent();
                GameObject* parentGO = AppEngine->GetSceneModule()->GetScene()->GetGameObjectByUID(parentUID);

                if (parentGO != nullptr && parentGO->IsEnabled())
                {
                    gameObject->SetEnabled(!gameObject->IsEnabled());
                }

                break;
            }
        }
    }
}

void PauseMenuScript::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator)
{
    targetState.AddMember("PanelToShow", rapidjson::Value(panelToShowName.c_str(), allocator), allocator);
}

void PauseMenuScript::Load(const rapidjson::Value& initialState)
{
    if (initialState.HasMember("PanelToShow") && initialState["PanelToShow"].IsString())
    {
        panelToShowName = initialState["PanelToShow"].GetString();
    }
}
