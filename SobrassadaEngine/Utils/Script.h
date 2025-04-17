#pragma once

// Here we only need to add the functions that are going to be used in the ScriptModule
class Script
{
  public:
    virtual ~Script() {}

    virtual bool Init()                  = 0;
    virtual void Update(float deltaTime) = 0;
    virtual void Inspector()             = 0;
};