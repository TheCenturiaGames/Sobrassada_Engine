#include "ResourceStateMachine.h"
#include "FileSystem.h"
#include "LibraryModule.h"
#include "Application.h"
#include "MetaModel.h"

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

