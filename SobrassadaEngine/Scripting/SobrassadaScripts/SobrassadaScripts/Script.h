#pragma once
#include <rapidjson/document.h>

class GameObject;
class Application;
#include <vector>

struct InspectorField
{
    enum class FieldType
    {
        Text,
        Float,
        Int,
        Bool,
        Vec2,
        Vec3,
        Vec4,
        Color
    };

    const char* name;
    FieldType type;
    void* data;
    float minValue;
    float maxValue;

    InspectorField(const char* name, FieldType type, void* data, float minValue, float maxValue)
        : name(name), type(type), data(data), minValue(minValue), maxValue(maxValue)
    {
    }
    InspectorField(const char* name, FieldType type, void* data)
        : name(name), type(type), data(data), minValue(0.0f), maxValue(1.0f)
    {
    }
    InspectorField(FieldType type, void* data) : name("No name"), type(type), data(data), minValue(0.0f), maxValue(1.0f)
    {
    }
};

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
    std::vector<InspectorField> fields;
};

extern Application* AppEngine;