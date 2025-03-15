#include "StateMachineImporter.h"
#include "Application.h"
#include "FileSystem.h"
#include "LibraryModule.h"
#include "ResourceManagement/Resources/ResourceStateMachine.h"

UID StateMachineImporter::ImportStateMachine(
    const std::vector<AnimationClipRef>& clips, const std::vector<AnimationState>& states,
    const std::vector<StateTransition>& transitions, const std::string& name
)
{
}

ResourceStateMachine* StateMachineImporter::LoadStateMachine(UID stateMachineUID)
{
}