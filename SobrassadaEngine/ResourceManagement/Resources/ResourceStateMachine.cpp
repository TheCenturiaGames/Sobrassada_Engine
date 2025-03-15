#include "ResourceStateMachine.h"

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

            for (const auto& clip : clips)
            {
                if (HashString(newName) != hashOld && clip.clipName == HashString(newName))
                {
                    GLOG("Clip with name '%s' already exists!", newName.c_str());
                    return false;
                }
            }
            clip.clipName = HashString(newName);
            
        }
        
    }

    return true;
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
