#pragma once

#include "FileSystem.h"
#include "ResourceManagement/Resources/ResourceStateMachine.h"

#include "Math/float4x4.h"

class ResourceStateMachine;

namespace StateMachineManager
{

    void Save(ResourceStateMachine* stateMachine, const std::string& path);
    void CopyMachine(const std::string& filePath, const std::string& name, const UID sourceUID);
    bool Load(ResourceStateMachine* stateMachine, const std::string& path);

}; // namespace StateMachineManager