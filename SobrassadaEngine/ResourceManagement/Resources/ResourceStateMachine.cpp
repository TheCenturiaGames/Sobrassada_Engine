#include "ResourceStateMachine.h"
#include "FileSystem.h"
#include "LibraryModule.h"
#include "Application.h"

ResourceStateMachine::ResourceStateMachine(UID uid, const std::string& name)
    : Resource(uid, name, ResourceType::Material)
{
}

void ResourceStateMachine::AddClip(UID clipUID, const std::string& name, bool loopFlag)
{
    HashString hashName(name);

    // Evitar duplicados
    for (const auto& clip : clips)
    {
        if (clip.clipName == hashName)
        {
            GLOG("Clip with name '%s' already exists!", name.c_str());
            return;
        }
    }

    Clip newClip;
    newClip.clipUID  = clipUID;
    newClip.clipName = hashName;
    newClip.loop     = loopFlag;
    clips.push_back(newClip);
}

bool ResourceStateMachine::RemoveClip(const std::string& name)
{
    HashString hashName(name);

    for (auto it = clips.begin(); it != clips.end(); it++)
    {
        if (it->clipName == hashName)
        {
            clips.erase(it);
            return true;
        }
    }

    return false;
}

bool ResourceStateMachine::EditClipInfo(
    const std::string& oldName, UID newUID, const std::string& newName, bool newLoop
)
{
    HashString hashOld(oldName);
    for (auto& clip : clips)
    {
        if (clip.clipName == hashOld)
        {
            clip.clipUID = newUID;
            clip.loop    = newLoop;

            if (HashString(newName) != hashOld)
            {
                for (const auto& clip : clips)
                {
                    if (clip.clipName == HashString(newName))
                    {
                        GLOG("Clip with name '%s' already exists!", newName.c_str());
                        return false;
                    }
                }
            }
            clip.clipName = HashString(newName);
            
        }
        
    }

    return true;
}

void ResourceStateMachine::AddState(const std::string& stateName, const std::string& clipName)
{
    HashString stateHash(stateName);

    for (const auto& state : states)
    {
        if (state.name == stateHash)
        {
            GLOG("State with name '%s' already exists!", stateName.c_str());
            return;
        }
    }

    if (!ClipExists(clipName))
    {
        GLOG("Clip with name '%s' does not exist. Cannot associate to state.", clipName.c_str());
        return;
    }

    State newState;
    newState.name     = stateHash;
    newState.clipName = HashString(clipName);

    states.push_back(newState);
}

bool ResourceStateMachine::RemoveState(const std::string& stateName)
{
    HashString stateHash(stateName);
    for (auto it = states.begin(); it != states.end(); ++it)
    {
        if (it->name == stateHash)
        {
            states.erase(it);
            return true;
        }
    }

    return false;
}

bool ResourceStateMachine::EditState(
    const std::string& oldStateName, const std::string& newStateName, const std::string& newClipName
)
{
    HashString hashOldState(oldStateName);
    for (auto& state : states)
    {
        if (state.name == hashOldState)
        {
            HashString hashNewState(newStateName);
            if (hashNewState != hashOldState)
            {
                for (const auto& s : states)
                {
                    if (s.name == hashNewState)
                    {
                        GLOG("Another state with name '%s' already exists!", newStateName.c_str());
                        return false;
                    }
                }
            }

            if (!ClipExists(newClipName))
            {
                GLOG("Clip with name '%s' does not exist. Cannot assign to state.", newClipName.c_str());
                return false;
            }

            state.name     = hashNewState;
            state.clipName = HashString(newClipName);
            return true;
        }
    }
    return false;
}

void ResourceStateMachine::AddTransition(
    const std::string& fromState, const std::string& toState, const std::string& trigger, unsigned interpolationTime
)
{
    HashString hashFrom(fromState);
    HashString hashTo(toState);

    bool fromExists = false, toExists = false;
    for (const auto& state : states)
    {
        if (state.name == hashFrom) fromExists = true;
        if (state.name == hashTo) toExists = true;
    }
    if (!fromExists || !toExists)
    {
        GLOG("Cannot add transition: one or both states do not exist.");
        return;
    }

    for (const auto& transition : transitions)
    {
        if (transition.fromState == hashFrom && transition.toState == hashTo)
        {
            GLOG("Transition from '%s' to '%s' already exists!", fromState.c_str(), toState.c_str());
            return;
        }
    }

    Transition newTransition;
    newTransition.fromState         = hashFrom;
    newTransition.toState           = hashTo;
    newTransition.triggerName       = HashString(trigger);
    newTransition.interpolationTime = interpolationTime;

    transitions.push_back(newTransition);

}

bool ResourceStateMachine::RemoveTransition(const std::string& fromState, const std::string& toState)
{
    HashString hashFrom(fromState);
    HashString hashTo(toState);

    for (auto it = transitions.begin(); it != transitions.end(); ++it)
    {
        if (it->fromState == hashFrom && it->toState == hashTo)
        {
            transitions.erase(it);
            return true;
        }
    }
    return false;
}

bool ResourceStateMachine::EditTransition(
    const std::string& fromState, const std::string& toState, const std::string& newTrigger, unsigned newInterpolationTime
)
{
    HashString hashFrom(fromState);
    HashString hashTo(toState);

    for (auto& transition : transitions)
    {
        if (transition.fromState == hashFrom && transition.toState == hashTo)
        {
            transition.triggerName       = HashString(newTrigger);
            transition.interpolationTime = newInterpolationTime;
            return true;
        }
    }
    return false;
}



const Clip* ResourceStateMachine::GetClip(const std::string& name) const
{
    HashString hashName(name);
    for (const auto& clip : clips)
    {
        if (clip.clipName == hashName) return &clip;
    }
    return nullptr;
}

const State* ResourceStateMachine::GetState(const std::string& name) const
{
    HashString hashName(name);
    for (const auto& state : states)
    {
        if (state.name == hashName) return &state;
    }
    return nullptr;
}

const Transition* ResourceStateMachine::GetTransition(const std::string& fromState, const std::string& toState) const
{
    HashString hashFrom(fromState);
    HashString hashTo(toState);
    for (const auto& transition : transitions)
    {
        if (transition.fromState == hashFrom && transition.toState == hashTo)
        {
            return &transition;
        }
    }
    return nullptr;
}


bool ResourceStateMachine::ClipExists(const std::string& clipName) const
{
    HashString hashClip(clipName);
    for (const auto& clip : clips)
    {
        if (clip.clipName == hashClip) return true;
    }
    return false;
}

void ResourceStateMachine::Save(const std::string& path) const
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

    UID finalStateMachineUID;
    UID stateMachineUID              = GenerateUID();
    finalStateMachineUID     = App->GetLibraryModule()->AssignFiletypeUID(stateMachineUID, FileType::StateMachine);
    std::string saveFilePath = STATEMACHINES_PATH + std::to_string(finalStateMachineUID) + STATEMACHINE_EXTENSION;

    unsigned int bytesWritten = (unsigned int)FileSystem::Save(saveFilePath.c_str(), buffer.data(), buffer.size(), true);

}

bool ResourceStateMachine::Load(const std::string& path)
{
    char* buffer  = nullptr;
    uint32_t size = FileSystem::Load(path.c_str(), &buffer);
    if (!buffer || size == 0) return false;

    char* cursor = buffer;

    clips.clear();
    states.clear();
    transitions.clear();

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
        cursor            += sizeof(bool);

        uint32_t nameSize  = 0;
        memcpy(&nameSize, cursor, sizeof(uint32_t));
        cursor += sizeof(uint32_t);
        std::string name(cursor, nameSize);
        cursor        += nameSize;
        clip.clipName  = HashString(name);

        clips.push_back(clip);
    }

    // --- Load States ---
    uint32_t numStates = 0;
    memcpy(&numStates, cursor, sizeof(uint32_t));
    cursor += sizeof(uint32_t);

    for (uint32_t i = 0; i < numStates; ++i)
    {
        State state;

        uint32_t nameSize = 0;
        memcpy(&nameSize, cursor, sizeof(uint32_t));
        cursor += sizeof(uint32_t);
        std::string stateName(cursor, nameSize);
        cursor            += nameSize;
        state.name         = HashString(stateName);

        uint32_t clipSize  = 0;
        memcpy(&clipSize, cursor, sizeof(uint32_t));
        cursor += sizeof(uint32_t);
        std::string clipName(cursor, clipSize);
        cursor         += clipSize;
        state.clipName  = HashString(clipName);

        states.push_back(state);
    }

    // --- Load Transitions ---
    uint32_t numTransitions = 0;
    memcpy(&numTransitions, cursor, sizeof(uint32_t));
    cursor += sizeof(uint32_t);

    for (uint32_t i = 0; i < numTransitions; ++i)
    {
        Transition t;

        uint32_t fromSize = 0;
        memcpy(&fromSize, cursor, sizeof(uint32_t));
        cursor += sizeof(uint32_t);
        std::string from(cursor, fromSize);
        cursor          += fromSize;
        t.fromState      = HashString(from);

        uint32_t toSize  = 0;
        memcpy(&toSize, cursor, sizeof(uint32_t));
        cursor += sizeof(uint32_t);
        std::string to(cursor, toSize);
        cursor               += toSize;
        t.toState             = HashString(to);

        uint32_t triggerSize  = 0;
        memcpy(&triggerSize, cursor, sizeof(uint32_t));
        cursor += sizeof(uint32_t);
        std::string trigger(cursor, triggerSize);
        cursor        += triggerSize;
        t.triggerName  = HashString(trigger);

        memcpy(&t.interpolationTime, cursor, sizeof(uint32_t));
        cursor += sizeof(uint32_t);

        transitions.push_back(t);
    }

    delete[] buffer;
    return true;
}