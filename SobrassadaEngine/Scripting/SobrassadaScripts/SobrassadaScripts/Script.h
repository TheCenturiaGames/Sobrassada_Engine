#pragma once

class GameObject;

class Script
{
  public:
    Script(GameObject* gameObject) : parent(gameObject) {}
    virtual ~Script() { parent = nullptr; }

    virtual bool Init()                  = 0;
    virtual void Update(float deltaTime) = 0;

  protected:
    GameObject* parent;
};