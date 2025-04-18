#pragma once
#include <rapidjson/document.h>

class GameObject;
class Application;

class Script
{
  public:
    Script(GameObject* gameObject) : parent(gameObject) {}
    virtual ~Script() { parent = nullptr; }

    virtual bool Init()                  = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Inspector()             = 0;

    virtual void SaveToJson(rapidjson::Value&, rapidjson::Document::AllocatorType&) const {}
    virtual void LoadFromJson(const rapidjson::Value&) {}

  protected:
    GameObject* parent;
};

extern Application* AppEngine;