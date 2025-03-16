#include "StateMachineManager.h"

#include "Application.h"
#include "LibraryModule.h"
#include "MetaModel.h"
#include "ResourceManagement/Resources/ResourceStateMachine.h"

#include "Math/Quat.h"
#include "Math/float4x4.h"
#include "prettywriter.h"
#include "stringbuffer.h"


namespace StateMachineManager
{
    void Save(
        const std::vector<Clip>& clips, const std::vector<State>& states, const std::vector<Transition>& transitions,
        const std::string& path, const UID sourceUID
    )
    {
        std::vector<char> buffer;

        // --- Save Clips ---
        uint32_t numClips = static_cast<uint32_t>(clips.size());
        buffer.insert(
            buffer.end(), reinterpret_cast<const char*>(&numClips),
            reinterpret_cast<const char*>(&numClips) + sizeof(uint32_t)
        );

        for (const auto& clip : clips)
        {
            buffer.insert(
                buffer.end(), reinterpret_cast<const char*>(&clip.clipUID),
                reinterpret_cast<const char*>(&clip.clipUID) + sizeof(UID)
            );
            buffer.insert(
                buffer.end(), reinterpret_cast<const char*>(&clip.loop),
                reinterpret_cast<const char*>(&clip.loop) + sizeof(bool)
            );

            uint32_t nameSize = static_cast<uint32_t>(clip.clipName.GetString().size());
            buffer.insert(
                buffer.end(), reinterpret_cast<const char*>(&nameSize),
                reinterpret_cast<const char*>(&nameSize) + sizeof(uint32_t)
            );
            buffer.insert(buffer.end(), clip.clipName.GetString().begin(), clip.clipName.GetString().end());
        }

        // --- Save States ---
        uint32_t numStates = static_cast<uint32_t>(states.size());
        buffer.insert(
            buffer.end(), reinterpret_cast<const char*>(&numStates),
            reinterpret_cast<const char*>(&numStates) + sizeof(uint32_t)
        );

        for (const auto& state : states)
        {
            uint32_t nameSize = static_cast<uint32_t>(state.name.GetString().size());
            buffer.insert(
                buffer.end(), reinterpret_cast<const char*>(&nameSize),
                reinterpret_cast<const char*>(&nameSize) + sizeof(uint32_t)
            );
            buffer.insert(buffer.end(), state.name.GetString().begin(), state.name.GetString().end());

            uint32_t clipNameSize = static_cast<uint32_t>(state.clipName.GetString().size());
            buffer.insert(
                buffer.end(), reinterpret_cast<const char*>(&clipNameSize),
                reinterpret_cast<const char*>(&clipNameSize) + sizeof(uint32_t)
            );
            buffer.insert(buffer.end(), state.clipName.GetString().begin(), state.clipName.GetString().end());
        }

        // --- Save Transitions ---
        uint32_t numTransitions = static_cast<uint32_t>(transitions.size());
        buffer.insert(
            buffer.end(), reinterpret_cast<const char*>(&numTransitions),
            reinterpret_cast<const char*>(&numTransitions) + sizeof(uint32_t)
        );

        for (const auto& t : transitions)
        {
            uint32_t fromSize = static_cast<uint32_t>(t.fromState.GetString().size());
            buffer.insert(
                buffer.end(), reinterpret_cast<const char*>(&fromSize),
                reinterpret_cast<const char*>(&fromSize) + sizeof(uint32_t)
            );
            buffer.insert(buffer.end(), t.fromState.GetString().begin(), t.fromState.GetString().end());

            uint32_t toSize = static_cast<uint32_t>(t.toState.GetString().size());
            buffer.insert(
                buffer.end(), reinterpret_cast<const char*>(&toSize),
                reinterpret_cast<const char*>(&toSize) + sizeof(uint32_t)
            );
            buffer.insert(buffer.end(), t.toState.GetString().begin(), t.toState.GetString().end());

            uint32_t triggerSize = static_cast<uint32_t>(t.triggerName.GetString().size());
            buffer.insert(
                buffer.end(), reinterpret_cast<const char*>(&triggerSize),
                reinterpret_cast<const char*>(&triggerSize) + sizeof(uint32_t)
            );
            buffer.insert(buffer.end(), t.triggerName.GetString().begin(), t.triggerName.GetString().end());

            buffer.insert(
                buffer.end(), reinterpret_cast<const char*>(&t.interpolationTime),
                reinterpret_cast<const char*>(&t.interpolationTime) + sizeof(uint32_t)
            );
        }

        const std::string stateMachineName = FileSystem::GetFileNameWithoutExtension(path);

       UID finalStateMachineUID;

        if (sourceUID == INVALID_UID)
        {
            UID stateMachineUID = GenerateUID();
            finalStateMachineUID  =
                App->GetLibraryModule()->AssignFiletypeUID(stateMachineUID, FileType::StateMachine);

            std::string assetPath = STATEMACHINES_ASSETS_PATH + stateMachineName + MODEL_EXTENSION;
            MetaModel meta(finalStateMachineUID, assetPath);
            meta.Save(stateMachineName, assetPath);
        }
        else
        {
            finalStateMachineUID = sourceUID;
        }
        std::string saveFilePath =
            STATEMACHINES_LIB_PATH + std::to_string(finalStateMachineUID) + STATEMACHINE_EXTENSION;
        FileSystem::Save(saveFilePath.c_str(), buffer.data(), buffer.size(), true);

        App->GetLibraryModule()->AddStateMachine(finalStateMachineUID, stateMachineName);
        App->GetLibraryModule()->AddName(stateMachineName, finalStateMachineUID);
        App->GetLibraryModule()->AddResource(saveFilePath, finalStateMachineUID);
    }

    void CopyMachine(const std::string& filePath, const std::string& name, const UID sourceUID)
    {
        std::string destination = STATEMACHINES_LIB_PATH + std::to_string(sourceUID) + STATEMACHINE_EXTENSION;
        FileSystem::Copy(filePath.c_str(), destination.c_str());

        App->GetLibraryModule()->AddStateMachine(sourceUID, name);
        App->GetLibraryModule()->AddName(name, sourceUID);
        App->GetLibraryModule()->AddResource(destination, sourceUID);
    }

   ResourceStateMachine* Load(UID stateMachineUID)
    {
        std::string path = App->GetLibraryModule()->GetResourcePath(stateMachineUID);
        if (path.empty())
        {
            GLOG("Invalid path for StateMachine UID %llu", stateMachineUID);
            return nullptr;
        }

        char* buffer  = nullptr;
        uint32_t size = FileSystem::Load(path.c_str(), &buffer);
        if (!buffer || size == 0)
        {
            GLOG("Failed to load StateMachine file: %s", path.c_str());
            return nullptr;
        }

        char* cursor = buffer;

        ResourceStateMachine* stateMachine =
            new ResourceStateMachine(stateMachineUID, FileSystem::GetFileNameWithoutExtension(path));

        // --- Load Clips ---
        uint32_t numClips = 0;
        memcpy(&numClips, cursor, sizeof(uint32_t));
        cursor += sizeof(uint32_t);

        for (uint32_t i = 0; i < numClips; ++i)
        {
            Clip clip;
            memcpy(&clip.clipUID, cursor, sizeof(UID));
            cursor += sizeof(UID);
            memcpy(&clip.loop, cursor, sizeof(bool));
            cursor += sizeof(bool);

            uint32_t nameSize;
            memcpy(&nameSize, cursor, sizeof(uint32_t));
            cursor += sizeof(uint32_t);
            std::string name(cursor, nameSize);
            cursor        += nameSize;
            clip.clipName  = HashString(name);

            stateMachine->clips.push_back(clip);
        }

        // --- Load States ---
        uint32_t numStates = 0;
        memcpy(&numStates, cursor, sizeof(uint32_t));
        cursor += sizeof(uint32_t);

        for (uint32_t i = 0; i < numStates; ++i)
        {
            State state;

            uint32_t nameSize;
            memcpy(&nameSize, cursor, sizeof(uint32_t));
            cursor += sizeof(uint32_t);
            std::string stateName(cursor, nameSize);
            cursor     += nameSize;
            state.name  = HashString(stateName);

            uint32_t clipNameSize;
            memcpy(&clipNameSize, cursor, sizeof(uint32_t));
            cursor += sizeof(uint32_t);
            std::string clipName(cursor, clipNameSize);
            cursor         += clipNameSize;
            state.clipName  = HashString(clipName);

            stateMachine->states.push_back(state);
        }

        // --- Load Transitions ---
        uint32_t numTransitions = 0;
        memcpy(&numTransitions, cursor, sizeof(uint32_t));
        cursor += sizeof(uint32_t);

        for (uint32_t i = 0; i < numTransitions; ++i)
        {
            Transition t;

            uint32_t fromSize;
            memcpy(&fromSize, cursor, sizeof(uint32_t));
            cursor += sizeof(uint32_t);
            std::string from(cursor, fromSize);
            cursor      += fromSize;
            t.fromState  = HashString(from);

            uint32_t toSize;
            memcpy(&toSize, cursor, sizeof(uint32_t));
            cursor += sizeof(uint32_t);
            std::string to(cursor, toSize);
            cursor    += toSize;
            t.toState  = HashString(to);

            uint32_t triggerSize;
            memcpy(&triggerSize, cursor, sizeof(uint32_t));
            cursor += sizeof(uint32_t);
            std::string trigger(cursor, triggerSize);
            cursor        += triggerSize;
            t.triggerName  = HashString(trigger);

            memcpy(&t.interpolationTime, cursor, sizeof(uint32_t));
            cursor += sizeof(uint32_t);

            stateMachine->transitions.push_back(t);
        }

        delete[] buffer;

        GLOG("StateMachine %llu loaded successfully from: %s", stateMachineUID, path.c_str());

        return stateMachine;
    }
} // namespace StateMachineManager
