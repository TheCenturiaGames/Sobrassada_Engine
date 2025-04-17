#pragma once

#include "Resource.h"

#include "Math/float2.h"
#include <string>
#include <vector>
#include "imgui.h"

struct HashString
{
    size_t hash = 0;
    std::string original;

    HashString() = default;
    HashString(const std::string& str)
    {
        original = str;
        hash     = std::hash<std::string> {}(str);
    }

    bool operator==(const HashString& other) const { return hash == other.hash; }
    bool operator!=(const HashString& other) const { return !(*this == other); }
    const std::string& GetString() const { return original; }
};

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

class ResourceStateMachine : public Resource
{
  public:
    std::vector<Clip> clips;
    std::vector<State> states;
    std::vector<Transition> transitions;
    std::vector<std::string> availableTriggers;

    ResourceStateMachine(UID uid, const std::string& name);
    ~ResourceStateMachine() override = default;

    void AddClip(UID animationResourceUID, const std::string& name, bool loop);
    bool RemoveClip(const std::string& name);
    bool EditClipInfo(const std::string& oldName, UID newUID, const std::string& newName, bool newLoop);

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

    const Clip* GetClip(const std::string& name) const;
    const State* GetState(const std::string& name) const;
    const Transition* GetTransition(const std::string& fromState, const std::string& toState) const;
    const State* GetDefaultState()
    {
        if (defaultStateIndex >= 0 && defaultStateIndex < (int)states.size()) return &states[defaultStateIndex];
        return nullptr;
    }
    const State* GetActiveState()
    {
        if (activeStateIndex >= 0 && activeStateIndex < (int)states.size()) return &states[activeStateIndex];
        return nullptr;
    }

    bool ClipExists(const std::string& clipName) const;
    
    void SetDefaultState(int state) { defaultStateIndex = state; }
    void SetActiveState(int state) { activeStateIndex = state; }

  private:
    int defaultStateIndex = -1;
    int activeStateIndex  = -1;
};