#pragma once

#include "Math/float2.h"
#include "Resource.h"
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
};

struct AnimationClip
{
    UID clipUID;
    HashString clipName;
    bool loop;
};

struct AnimationState
{
    HashString name;
    HashString clipName;
};

struct StateTransition
{
    HashString fromState;
    HashString toState;
    HashString triggerName;
    unsigned interpolationTime;
};

class ResourceStateMachine : public Resource
{
  public:
    ResourceStateMachine(UID uid, const std::string& name);
    ~ResourceStateMachine() override = default;

    std::vector<AnimationClip> clips;
    std::vector<AnimationState> states;
    std::vector<StateTransition> transitions;
};