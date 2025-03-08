#include "AnimController.h"
#include "ResourcesModule.h"
#include "Application.h"
#include <chrono>

AnimController::AnimController() : resource(0), currentTime(0), loop(false)
{
}

void AnimController::Play(UID newResource, bool shouldLoop)
{
    resource    = newResource;
    currentTime = 0;
    loop        = shouldLoop;
    playAnimation = true;
    startTime     = clock();
}

void AnimController::Stop()
{
    resource    = 0;
    currentTime = 0;
    loop        = false;
    playAnimation = false;
}

update_status AnimController::Update(float deltaTime)
{

    if (resource == 0) return UPDATE_CONTINUE;
    

    if (playAnimation == false) return UPDATE_CONTINUE;

    ResourceAnimation* animation =
        static_cast<ResourceAnimation*>(App->GetResourcesModule()->RequestResource(resource));
    if (!animation) return UPDATE_CONTINUE;

    float duration = animation->GetDuration();
    if (duration <= 0.0f) return UPDATE_CONTINUE;

    float delta_time = (clock() - startTime) * 1000.0f / CLOCKS_PER_SEC;
    currentTime      = delta_time / 1000.0f;


    if (currentTime > duration)
    {
        if (loop)
        {
            startTime   = clock();
            currentTime = 0; 
        }
        else
        {
            currentTime = duration; 
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
                animChannel->posTimeStamps.get(), animChannel->posTimeStamps.get() + animChannel->numPositions,
                currentTime
            );

            size_t index = upper - animChannel->posTimeStamps.get();

            if (index == 0)
            {
                pos = animChannel->positions[0];
            }
            else if (index >= animChannel->numPositions)
            {
                pos = animChannel->positions[animChannel->numPositions - 1];
            }
            else
            {
                float lambda = (currentTime - animChannel->posTimeStamps[index - 1]) /
                               (animChannel->posTimeStamps[index] - animChannel->posTimeStamps[index - 1]);
                float3 interpolation =
                    PosInterpolate(animChannel->positions[index], animChannel->positions[index - 1], lambda);
                GLOG(
                    "CURRENTTime: %f y position: (%f,%f,%f)", currentTime, interpolation.x, interpolation.y,
                    interpolation.z
                );
            }
        }
    }
    if (animChannel->rotations && animChannel->numRotations > 0)
    {
        if (animChannel->numRotations == 1)
        {
            rot = animChannel->rotations[0];
        }
        else
        {
            auto upper = std::upper_bound(
                animChannel->rotTimeStamps.get(), animChannel->rotTimeStamps.get() + animChannel->numRotations,
                currentTime
            );

            size_t index = upper - animChannel->rotTimeStamps.get();

            if (index == 0)
            {
                rot = animChannel->rotations[0];
            }
            else if (index >= animChannel->numRotations)
            {
                rot = animChannel->rotations[animChannel->numRotations - 1];
            }
            else
            {
                float lambda = (currentTime - animChannel->rotTimeStamps[index - 1]) /
                               (animChannel->rotTimeStamps[index] - animChannel->rotTimeStamps[index - 1]);
                Quat interpolation =
                    QuatInterpolate(animChannel->rotations[index], animChannel->rotations[index - 1], lambda);
                GLOG(
                    "CURRENTTime: %f y rotation: (%f,%f,%f,%f)", currentTime, interpolation.x, interpolation.y,
                    interpolation.z, interpolation.w
                );
            }
        }
    }
}

float3 AnimController::PosInterpolate(const float3& first, const float3& second, float lambda)
{
    return first * (1.0f - lambda) + second * lambda;
}

Quat AnimController::QuatInterpolate(const Quat& first, const Quat& second, float lambda)
{
    Quat result;
    float dot = first.Dot(second);
    if (dot >= 0.0f)
    {
        result.x = first.x * (1.0f - lambda) + second.x * lambda;
        result.y = first.y * (1.0f - lambda) + second.y * lambda;
        result.z = first.z * (1.0f - lambda) + second.z * lambda;
        result.w = first.w * (1.0f - lambda) + second.w * lambda;
    }
    else
    {
        result.x = first.x * (1.0f - lambda) - second.x * lambda;
        result.y = first.y * (1.0f - lambda) - second.y * lambda;
        result.z = first.z * (1.0f - lambda) - second.z * lambda;
        result.w = first.w * (1.0f - lambda) - second.w * lambda;
    }
    result.Normalize();
    return result;
}