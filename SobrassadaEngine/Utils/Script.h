#pragma once
#include <rapidjson/document.h>

// Here we only need to add the functions that are going to be used in the ScriptModule
class Script
{
  public:
    virtual ~Script() {}

    virtual bool Init()                  = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Inspector()             = 0;

    virtual void SaveToJson(rapidjson::Value&, rapidjson::Document::AllocatorType&) const {}
    virtual void LoadFromJson(const rapidjson::Value&) {}
};