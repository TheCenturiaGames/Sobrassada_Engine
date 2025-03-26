#pragma once

#include <Globals.h>

#include <string>
#include <vector>

class ResourceStateMachine;
struct Clip;
struct State;
struct Transition;

namespace StateMachineManager
{
    void Save(ResourceStateMachine* stateMachine);
    void CopyMachine(const std::string& filePath, const std::string& name, const UID sourceUID);
    ResourceStateMachine* Load(UID stateMachineUID);

}; // namespace StateMachineManager