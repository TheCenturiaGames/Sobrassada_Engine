#pragma once
#include "Script.h"

#include <rapidjson/document.h>
#include <string>
#include <list>
#include "Utils/Delegate.h"


class ButtonScript : public Script
{
  public:
    ButtonScript(GameObject* parent) : Script(parent) {}
    ~ButtonScript() override;

    bool Init() override;
    void Update(float deltaTime) override;
    void Inspector() override;

    void OnClick();
    std::string GetCurrentPanelName() const;
    void SaveToJson(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const override;
    void LoadFromJson(const rapidjson::Value& value) override;

  private:
    std::string panelToHideName = "MainMenuPanel";
    std::string panelToShowName = "OptionsPanel";

    std::list<Delegate<void>>::iterator delegateID;
    bool hasRegisteredCallback = false;
};
