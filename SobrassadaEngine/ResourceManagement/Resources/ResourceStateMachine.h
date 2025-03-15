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
    bool operator!=(const HashString& other) const { return !(*this == other); }
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
    ResourceStateMachine(UID uid, const std::string& name);
    ~ResourceStateMachine() override = default;

    void AddClip(UID clipUID, const std::string& name, bool loop);
    bool RemoveClip(const std::string& name);
    bool EditClipInfo(const std::string& oldName, UID newUID, const std::string& newName, bool newLoop);

    const Clip* GetClip(const std::string& name) const;

  private:

    std::vector<Clip> clips;
    std::vector<State> states;
    std::vector<Transition> transitions;

    

};