#pragma once
#include "Script.h"

class PressAnyKeyScript : public Script
{
  public:
    PressAnyKeyScript(GameObject* parent) : Script(parent) {}

    bool Init() override;
    void Update(float deltaTime) override;
    void Inspector() override {}
    void SaveToJson(rapidjson::Value&, rapidjson::Document::AllocatorType&) const override {}
    void LoadFromJson(const rapidjson::Value&) override {}

  private:
    bool inputReady = false;
};
