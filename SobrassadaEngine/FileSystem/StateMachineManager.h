#pragma once

#include "Globals.h"

#include <string>
#include <vector>

class ResourceStateMachine;

namespace StateMachineManager
{
    UID SaveStateMachine(const ResourceStateMachine* stateMachine, bool override);
    void CopyMachine(const std::string& filePath, const std::string& targetFilePath, const std::string& name, const UID sourceUID);
    ResourceStateMachine* LoadStateMachine(UID stateMachineUID);
    std::vector<std::string> GetAllStateMachineNames();
    UID GetStateMachineUID(const std::string& stateMachineName);

}; // namespace StateMachineManager