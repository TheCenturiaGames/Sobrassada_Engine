#pragma once

class EngineTimer
{
  public:
    EngineTimer();
    virtual ~EngineTimer();

    virtual void Start();
    virtual float Tick();

    float GetTime() const { return time; }
    float GetDeltaTime() const { return deltaTime; }

  protected:
    virtual float TicksSinceStartup() const;

  protected:
    bool isEnabled;
    float time;
    float startTime;
    float deltaTime;
};