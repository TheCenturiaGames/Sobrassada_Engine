#pragma once

#include "Resource.h"
#include "HashString.h"

#include "Math/float2.h"
#include "imgui.h"
#include <string>
#include <vector>


struct Clip
{
    UID animationResourceUID;
    HashString clipName;
    bool loop;
};

struct State
{
    HashString name;
    HashString clipName;
    ImVec2 position;
};

struct Transition
{
    HashString fromState;
    HashString toState;
    HashString triggerName;
    unsigned interpolationTime;
};

class SOBRASADA_API_ENGINE ResourceStateMachine : public Resource
{
  public:
    ResourceStateMachine(UID uid, const std::string& name);
    ~ResourceStateMachine() override = default;

    void AddClip(UID animationResourceUID, const std::string& name, bool loop);
    bool RemoveClip(const std::string& name);
    bool EditClipInfo(const std::string& oldName, UID newUID, const std::string& newName, bool newLoop);
    bool ClipExists(const std::string& clipName) const;

    void AddState(const std::string& stateName, const std::string& clipName);
    bool RemoveState(const std::string& stateName);
    bool EditState(const std::string& oldStateName, const std::string& newStateName, const std::string& newClipName);

    void AddTransition(
        const std::string& fromState, const std::string& toState, const std::string& trigger, unsigned interpolationTime
    );
    bool RemoveTransition(const std::string& fromState, const std::string& toState);
    bool EditTransition(
        const std::string& fromState, const std::string& toState, const std::string& newTrigger,
        unsigned newInterpolationTime
    );

    bool UseTrigger(const std::string& triggerName);

    const Clip* GetClip(const std::string& name) const;
    const State* GetState(const std::string& name) const;
    const Transition* GetTransition(const std::string& fromState, const std::string& toState) const;
    const State* GetDefaultState() const
    {
        if (defaultStateIndex >= 0 && defaultStateIndex < (int)states.size()) return &states[defaultStateIndex];
        return nullptr;
    }
    const State* GetActiveState() const
    {
        if (activeStateIndex >= 0 && activeStateIndex < (int)states.size()) return &states[activeStateIndex];
        return nullptr;
    }

    void SetDefaultState(int state) { defaultStateIndex = state; }
    void SetActiveState(int state) { activeStateIndex = state; }

  public:
    std::vector<Clip> clips;
    std::vector<State> states;
    std::vector<Transition> transitions;
    std::vector<std::string> availableTriggers;

  private:
    int defaultStateIndex = -1;
    int activeStateIndex  = -1;
};