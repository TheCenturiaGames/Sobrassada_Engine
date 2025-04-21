#pragma once

#include "Script.h"
#include <string>

class PressAnyKeyScript : public Script
{
  public:
    PressAnyKeyScript(GameObject* parent) : Script(parent) {}

    bool Init() override;
    void Update(float deltaTime) override;
    void Inspector() override;
    void SaveToJson(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const override;
    void LoadFromJson(const rapidjson::Value& value) override;

  private:
    std::string nextGameObjectName;
};
