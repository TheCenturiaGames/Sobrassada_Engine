#include "StateMachineManager.h"

#include "Application.h"
#include "FileSystem.h"
#include "LibraryModule.h"
#include "MetaModel.h"
#include "ProjectModule.h"
#include <ResourceStateMachine.h>

#include "Math/Quat.h"
#include "prettywriter.h"
#include "stringbuffer.h"
#include <Math/float4x4.h>
#include <filesystem>
#include <queue>


namespace StateMachineManager
{
    UID Save(ResourceStateMachine* resource, bool override
    )
    {
        // Create doc Json
        rapidjson::Document doc;
        doc.SetObject();
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        rapidjson::Value stateMachineJSON(rapidjson::kObjectType);

        UID uid              = GenerateUID();
        std::string savePath = STATEMACHINES_LIB_PATH + std::string("StateMachine") + STATEMACHINE_EXTENSION;
        UID stateMachineUID =
            override ? resource->GetUID() : App->GetLibraryModule()->AssignFiletypeUID(uid, FileType::StateMachine);
        savePath = App->GetProjectModule()->GetLoadedProjectPath() + STATEMACHINES_LIB_PATH +
                   std::to_string(stateMachineUID) + STATEMACHINE_EXTENSION;
        std::string stateName = resource->GetName();

        stateMachineJSON.AddMember("UID", stateMachineUID, allocator);
        stateMachineJSON.AddMember("Name", rapidjson::Value(stateName.c_str(), allocator), allocator);

         //Clips
        rapidjson::Value clipsArray(rapidjson::kArrayType);
        for (const auto& clip : resource->clips)
        {
            rapidjson::Value clipJSON(rapidjson::kObjectType);
            clipJSON.AddMember("ClipUID", clip.clipUID, allocator);
            clipJSON.AddMember("ClipName", rapidjson::Value(clip.clipName.GetString().c_str(), allocator), allocator);
            clipJSON.AddMember("Loop", clip.loop, allocator);

            clipsArray.PushBack(clipJSON, allocator);
        }
        stateMachineJSON.AddMember("Clips", clipsArray, allocator);

        //States
        rapidjson::Value statesArray(rapidjson::kArrayType);
        for (const auto& state : resource->states)
        {
            rapidjson::Value stateJSON(rapidjson::kObjectType);
            stateJSON.AddMember("StateName", rapidjson::Value(state.name.GetString().c_str(), allocator), allocator);
            stateJSON.AddMember("ClipName", rapidjson::Value(state.clipName.GetString().c_str(), allocator), allocator);

            statesArray.PushBack(stateJSON, allocator);
        }
        stateMachineJSON.AddMember("States", statesArray, allocator);

        //Transitions
        rapidjson::Value transitionsArray(rapidjson::kArrayType);
        for (const auto& transition : resource->transitions)
        {
            rapidjson::Value transitionJSON(rapidjson::kObjectType);
            transitionJSON.AddMember(
                "FromState", rapidjson::Value(transition.fromState.GetString().c_str(), allocator), allocator
            );
            transitionJSON.AddMember(
                "ToState", rapidjson::Value(transition.toState.GetString().c_str(), allocator), allocator
            );
            transitionJSON.AddMember(
                "Trigger", rapidjson::Value(transition.triggerName.GetString().c_str(), allocator), allocator
            );
            transitionJSON.AddMember("InterpolationTime", transition.interpolationTime, allocator);

            transitionsArray.PushBack(transitionJSON, allocator);
        }
        stateMachineJSON.AddMember("Transitions", transitionsArray, allocator);

        doc.AddMember("StateMachine", stateMachineJSON, allocator);
        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        std::string machinePath = STATEMACHINES_LIB_PATH + std::to_string(stateMachineUID) + STATEMACHINE_EXTENSION;
        unsigned int bytesWritten = (unsigned int
        )FileSystem::Save(machinePath.c_str(), buffer.GetString(), (unsigned int)buffer.GetSize(), false);
   
        if (bytesWritten == 0)
        {
            GLOG("Failed to save state machine file: %s", machinePath.c_str());
            return INVALID_UID;
        }

        App->GetLibraryModule()->AddStateMachine(stateMachineUID, stateName);
        App->GetLibraryModule()->AddName(stateName, stateMachineUID);
        App->GetLibraryModule()->AddResource(machinePath, stateMachineUID);

        return stateMachineUID;

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
