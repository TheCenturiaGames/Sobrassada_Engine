#pragma once

#include "FileSystem.h"
#include "ResourceManagement/Resources/ResourceStateMachine.h"

#include "Math/float4x4.h"

namespace StateMachineManager
{

    void Save(
        const std::vector<Clip>& clips, const std::vector<State>& states, const std::vector<Transition>& transitions,
        const std::string& path, const UID sourceUID
    );
    void CopyMachine(const std::string& filePath, const std::string& name, const UID sourceUID);
    ResourceStateMachine* Load(UID stateMachineUID);

}; // namespace StateMachineManager