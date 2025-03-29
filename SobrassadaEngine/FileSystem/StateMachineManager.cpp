#include "StateMachineManager.h"

#include "Application.h"
#include "FileSystem.h"
#include "LibraryModule.h"
#include "MetaStateMachine.h"
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

         // Create meta file
        std::string assetPath = STATEMACHINES_LIB_PATH + stateName + STATEMACHINE_EXTENSION;
        MetaStateMachine meta(stateMachineUID, assetPath);
        meta.Save(stateName, assetPath);
        assetPath = App->GetProjectModule()->GetLoadedProjectPath() + STATEMACHINES_LIB_PATH + stateName +
                    STATEMACHINE_EXTENSION;
        // Save in assets
        unsigned int bytesWritten = (unsigned int
        )FileSystem::Save(assetPath.c_str(), buffer.GetString(), (unsigned int)buffer.GetSize(), false);
        if (bytesWritten == 0)
        {
            GLOG("Failed to save stateMachine file: %s", assetPath.c_str());
            return INVALID_UID;
        }

        std::string machinePath = STATEMACHINES_LIB_PATH + std::to_string(stateMachineUID) + STATEMACHINE_EXTENSION;
        bytesWritten = (unsigned int
        )FileSystem::Save(machinePath.c_str(), buffer.GetString(), (unsigned int)buffer.GetSize(), false);
   
        if (bytesWritten == 0)
        {
            GLOG("Failed to save state machine file: %s", machinePath.c_str());
            return INVALID_UID;
        }

        App->GetLibraryModule()->AddStateMachine(stateMachineUID, stateName);
        App->GetLibraryModule()->AddName(stateName, stateMachineUID);
        App->GetLibraryModule()->AddResource(machinePath, stateMachineUID);

       // Load(stateMachineUID);

        return stateMachineUID;

    }

    void CopyMachine(const std::string& filePath, const std::string& targetFilePath, const std::string& name, const UID sourceUID)
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

        rapidjson::Document doc;
        bool loaded = FileSystem::LoadJSON(path.c_str(), doc);
        if (!loaded || !doc.HasMember("StateMachine"))
        {
            GLOG("Failed to parse StateMachine JSON: %s", path.c_str());
            return nullptr;
        }

        const rapidjson::Value& stateMachineJSON = doc["StateMachine"];

        UID uid                                  = stateMachineJSON["UID"].GetUint64();
        std::string name                         = stateMachineJSON["Name"].GetString();


        ResourceStateMachine* stateMachine = new ResourceStateMachine(uid, name);

        //Clips
        if (stateMachineJSON.HasMember("Clips") && stateMachineJSON["Clips"].IsArray())
        {
            for (const auto& clipJSON : stateMachineJSON["Clips"].GetArray())
            {
                Clip clip;
                clip.clipUID  = clipJSON["ClipUID"].GetUint64();
                clip.clipName = HashString(clipJSON["ClipName"].GetString());
                clip.loop     = clipJSON["Loop"].GetBool();

                stateMachine->clips.push_back(clip);
            }
        }

        //States
        if (stateMachineJSON.HasMember("States") && stateMachineJSON["States"].IsArray())
        {
            for (const auto& stateJSON : stateMachineJSON["States"].GetArray())
            {
                State state;
                state.name     = HashString(stateJSON["StateName"].GetString());
                state.clipName = HashString(stateJSON["ClipName"].GetString());

                stateMachine->states.push_back(state);
            }
        }

        //Transitions
        if (stateMachineJSON.HasMember("Transitions") && stateMachineJSON["Transitions"].IsArray())
        {
            for (const auto& transitionJSON : stateMachineJSON["Transitions"].GetArray())
            {
                Transition t;
                t.fromState         = HashString(transitionJSON["FromState"].GetString());
                t.toState           = HashString(transitionJSON["ToState"].GetString());
                t.triggerName       = HashString(transitionJSON["Trigger"].GetString());
                t.interpolationTime = transitionJSON["InterpolationTime"].GetFloat();

                stateMachine->transitions.push_back(t);
            }
        }

        GLOG("StateMachine %llu loaded successfully from: %s", stateMachineUID, path.c_str());

        return stateMachine;
    }

} // namespace StateMachineManager
