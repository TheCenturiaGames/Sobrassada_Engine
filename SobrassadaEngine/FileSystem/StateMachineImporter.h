#pragma once

#include "Globals.h"
#include <string>
#include <vector>

struct AnimationClipRef;
struct AnimationState;
struct StateTransition;
class ResourceStateMachine;

namespace StateMachineImporter
{
    UID ImportStateMachine(
        const std::vector<AnimationClipRef>& clips, const std::vector<AnimationState>& states,
        const std::vector<StateTransition>& transitions, const std::string& name
    );

    ResourceStateMachine* LoadStateMachine(UID stateMachineUID);
} // namespace StateMachineImporter