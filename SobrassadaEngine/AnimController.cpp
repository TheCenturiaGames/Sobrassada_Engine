#include "AnimController.h"
#include "ResourcesModule.h"
#include "Application.h"

AnimController::AnimController() : resource(0), currentTime(0), loop(false)
{
}

void AnimController::Play(UID newResource, bool shouldLoop)
{
    resource    = newResource;
    currentTime = 12.56;
    loop        = shouldLoop;
    playAnimation = true;
    ResourceAnimation* animation =
        static_cast<ResourceAnimation*>(App->GetResourcesModule()->RequestResource(resource));

    if (!animation)
    {
        GLOG("ERROR: No se encontró la animación con UID: %llu", resource);
        return; // Evita el crash
    }

    for (const auto& channelPair : animation->channels)
    {
        const std::string& nodeName = channelPair.first;
        const Channel& animChannel  = channelPair.second;

        uint32_t posIndex           = 0;
        uint32_t rotIndex           = 0;

        GLOG(
            "Node: %s, Total Positions: %u, Total Rotations: %u", nodeName.c_str(), animChannel.numPositions,
            animChannel.numRotations
        );

        while (posIndex < animChannel.numPositions || rotIndex < animChannel.numRotations)
        {
            if (posIndex < animChannel.numPositions &&
                (rotIndex >= animChannel.numRotations ||
                 animChannel.posTimeStamps[posIndex] <= animChannel.rotTimeStamps[rotIndex]))
            {
                const float timestamp  = animChannel.posTimeStamps[posIndex];
                const float3& position = animChannel.positions[posIndex];

                std::string logMessage =
                    "Keyframe " + std::to_string(posIndex) + ": Time: " + std::to_string(timestamp);
                logMessage += ", Translation: (" + std::to_string(position.x) + ", " + std::to_string(position.y) +
                              ", " + std::to_string(position.z) + ")";

                if (rotIndex < animChannel.numRotations &&
                    animChannel.posTimeStamps[posIndex] == animChannel.rotTimeStamps[rotIndex])
                {
                    const Quat& rotation = animChannel.rotations[rotIndex];
                    logMessage += ", Rotation: (" + std::to_string(rotation.x) + ", " + std::to_string(rotation.y) +
                                  ", " + std::to_string(rotation.z) + ", " + std::to_string(rotation.w) + ")";
                    rotIndex++;
                }

                GLOG("%s", logMessage.c_str());

                posIndex++;
            }
            else if (rotIndex < animChannel.numRotations)
            {
                const float timestamp = animChannel.rotTimeStamps[rotIndex];
                const Quat& rotation  = animChannel.rotations[rotIndex];

                std::string logMessage =
                    "Keyframe " + std::to_string(rotIndex) + ": Time: " + std::to_string(timestamp);
                logMessage += ", Rotation: (" + std::to_string(rotation.x) + ", " + std::to_string(rotation.y) + ", " +
                              std::to_string(rotation.z) + ", " + std::to_string(rotation.w) + ")";

                GLOG("%s", logMessage.c_str());

                rotIndex++;
            }
        }
    }
}

void AnimController::Stop()
{
    resource    = 0;
    currentTime = 0;
    loop        = false;
    playAnimation = false;
}

update_status AnimController::Update()
{
    if (resource == 0) return UPDATE_CONTINUE; 

    ResourceAnimation* animation =
        static_cast<ResourceAnimation*>(App->GetResourcesModule()->RequestResource(resource));
    if (!animation) return UPDATE_CONTINUE;

    float duration = animation->GetDuration();
    if (duration <= 0.0f) return UPDATE_CONTINUE;

   // currentTime += deltatime;

    if (currentTime > duration)
    {
        if (loop)
        {
            
        }
        else
        {
            //currentTime = duration; 
        }
    }

    return UPDATE_CONTINUE;
}

void AnimController::GetTransform(const std::string& nodeName, float3& pos, Quat& rot)
{
    ResourceAnimation* animation =
        static_cast<ResourceAnimation*>(App->GetResourcesModule()->RequestResource(resource));

    Channel* animChannel = animation->GetChannel(nodeName);

    GLOG("POSITIONS of channel %s: %d", nodeName.c_str(), animChannel->numPositions);

    if (animChannel->positions && animChannel->numPositions > 0)
    {
        if (animChannel->numPositions == 1)
        {
            pos = animChannel->positions[0]; 
        }
        else
        {
            auto upper = std::upper_bound(
                animChannel->posTimeStamps.get(), animChannel->posTimeStamps.get() + animChannel->numPositions, currentTime
            );

            size_t index = upper - animChannel->posTimeStamps.get();

             if (index == 0)
            {
               pos = animChannel->positions[0]; 
            }
            else if (index >= animChannel->numPositions)
            {
                pos =
                    animChannel->positions[animChannel->numPositions - 1]; 
            }
            else
            {
                float lambda = (currentTime - animChannel->posTimeStamps[index-1]) /
                               (animChannel->posTimeStamps[index] - animChannel->posTimeStamps[index-1]);
                float3 interpolation =
                    PosInterpolate(animChannel->positions[index], animChannel->positions[index - 1], lambda);
                GLOG(
                    "CURRENTTime: %f y position: (%f,%f,%f)", currentTime,interpolation.x, interpolation.y,
                    interpolation.z
                );
            }
        }
    }
}