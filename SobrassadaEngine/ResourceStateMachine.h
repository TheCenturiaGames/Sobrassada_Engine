#pragma once

#include "Math/float2.h"
#include "Resource.h"
#include <vector>

struct AnimationClip
{
    UID clipUID;          
    std::string clipName; 
};
struct AnimationState
{
    std::string name;
    UID clipUID;
    float2 positionInGraph; 
};
struct StateTransition
{
    std::string fromState;
    std::string toState;
    std::string conditionParam; 
    bool hasCondition = false;  
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