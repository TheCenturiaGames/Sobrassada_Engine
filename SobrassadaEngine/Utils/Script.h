#pragma once
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
};

// Here we only need to add the functions that are going to be used in the ScriptModule
class Script
{
  public:
    virtual ~Script() {}

    virtual bool Init()                  = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Inspector()             = 0;
};