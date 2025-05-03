#pragma once
#include "Script.h"

class TileFloatScript : public Script
{
  public:
    TileFloatScript(GameObject* parent) : Script(parent) {};
    bool Init() override;
    void Update(float deltaTime) override;
    void Inspector() override;
    void Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) override;
    void Load(const rapidjson::Value& initialState) override;

  private:
    float speed           = 0.5f;
    float maxRiseDistance = 15.0f;
    float initialY        = 0.0f;
};