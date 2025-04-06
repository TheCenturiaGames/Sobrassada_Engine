#pragma once

#include "Resource.h"

#include "Math/float2.h"
#include <string>
#include <vector>

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
    UID clipUID;
    HashString clipName;
    bool loop;
};

struct State
{
    HashString name;
    HashString clipName;
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

    ResourceStateMachine(UID uid, const std::string& name);
    ~ResourceStateMachine() override = default;

    void AddClip(UID clipUID, const std::string& name, bool loop);
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

    bool ClipExists(const std::string& clipName) const;

    void SetUID(UID uid) { stateMachineUID = uid; };

  private:
    UID stateMachineUID;
};