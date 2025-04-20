#pragma once
#include "Script.h"
#include <string>
#include <vector>
class GameObject;

class OptionsMenuSwitcherScript : public Script
{
  public:
    
    OptionsMenuSwitcherScript(GameObject* parent) : Script(parent) {}

    bool Init() override;
    void Update(float deltaTime) override;
    void Inspector() override;
    void SaveToJson(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const override;
    void LoadFromJson(const rapidjson::Value& value) override;

  private:
    std::vector<std::string> panelNames = {
        "OptionsKeyboardPanel", "OptionsControllerPanel", "OptionsAudioPanel", "OptionsVideoPanel"
    };

    int currentIndex = 0;
    bool initialized = false;

    void ShowOnlyCurrentPanel();
    GameObject* FindPanelByName(const std::string& name) const;
};
