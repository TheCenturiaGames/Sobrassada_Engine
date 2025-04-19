#pragma once

#include "Script.h"
#include <string>

class PauseMenuScript : public Script
{
  public:
    PauseMenuScript(GameObject* parent) : Script(parent) {}

    bool Init() override;
    void Update(float deltaTime) override;
    void Inspector() override {};

    void SaveToJson(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const override
    {
        value.AddMember("PanelToShow", rapidjson::Value(panelToShowName.c_str(), allocator), allocator);
    }

    void LoadFromJson(const rapidjson::Value& value) override
    {
        if (value.HasMember("PanelToShow") && value["PanelToShow"].IsString())
        {
            panelToShowName = value["PanelToShow"].GetString();
        }
    }

  private:
    std::string panelToShowName = "PauseMenuPanel";
};
