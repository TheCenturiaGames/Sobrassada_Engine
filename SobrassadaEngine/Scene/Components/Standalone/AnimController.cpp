#include "AnimController.h"

#include "Application.h"
#include "EngineTimer.h"
#include "ResourceAnimation.h"
#include "ResourcesModule.h"

AnimController::AnimController()
    : resource(0), currentTime(0), loop(false), playAnimation(false), playbackSpeed(1.0f), animation(nullptr)
{
}

AnimController::~AnimController()
{
    App->GetResourcesModule()->ReleaseResource(animation);
}

void AnimController::Play(UID newResource, bool shouldLoop)
{
    resource      = newResource;
    currentTime   = 0;
    loop          = shouldLoop;
    animation     = static_cast<ResourceAnimation*>(App->GetResourcesModule()->RequestResource(resource));
    playAnimation = true;
}

void AnimController::Stop()
{
    playAnimation = false;
    currentTime   = 0.0f;

    if (animation != nullptr)
    {
        App->GetResourcesModule()->ReleaseResource(animation);
        animation = nullptr;
    }
}

Quat AnimController::Interpolate(Quat& first, Quat& second, float lambda)
{
    if (first.Dot(second) >= 0.0f) return Quat::Lerp(first, second, lambda).Normalized();
    else return Quat::Lerp(first, second.Neg(), lambda).Normalized();
}

update_status AnimController::Update(float deltaTime)
{
    if (!playAnimation || resource == 0) return UPDATE_CONTINUE;

    deltaTime          *= playbackSpeed;

    /*GLOG(
        "Raw delta time: %f, Capped delta: %f, Playback speed: %f", App->GetEngineTimer()->GetTime(), rawDeltaTime,
        playbackSpeed
    );*/

    float previousTime  = currentTime;
    currentTime        += deltaTime;

    if (animation == nullptr) return UPDATE_CONTINUE;

    const float duration = animation->GetDuration();
    // GLOG("Animation time update: %f -> %f (duration: %f)", previousTime, currentTime, duration);

    if (currentTime > duration)
    {
        if (loop)
        {
            currentTime = fmod(currentTime, duration);
            // GLOG("Animation looped: new time = %f", currentTime);
        }
        else
        {
            currentTime   = duration;
            // GLOG("Animation reached end: time = %f", currentTime);
            playAnimation = false;

            if (animation != nullptr)
            {
                App->GetResourcesModule()->ReleaseResource(animation);
                animation = nullptr;
            }
        }
    }

    return UPDATE_CONTINUE;
}

void AnimController::GetTransform(const std::string& nodeName, float3& pos, Quat& rot)
{
    if (!playAnimation || resource == 0 || animation == nullptr) return;

    Channel* animChannel = animation->GetChannel(nodeName);
    if (animChannel == nullptr) return;

    float animDuration = animation->GetDuration();
    // GLOG("Animation duration: %f, Current time: %f", animDuration, currentTime);

    if (animChannel->numPositions > 0)
    {
        if (animChannel->numPositions == 1)
        {
            pos = animChannel->positions[0];
            // GLOG("Single position keyframe: (%f,%f,%f)", pos.x, pos.y, pos.z);
        }
        else
        {
            size_t nextIndex = 0;
            while (nextIndex < animChannel->numPositions && animChannel->posTimeStamps[nextIndex] <= currentTime)
            {
                nextIndex++;
            }

            size_t prevIndex = (nextIndex > 0) ? nextIndex - 1 : 0;

            if (nextIndex >= animChannel->numPositions)
            {
                pos = animChannel->positions[animChannel->numPositions - 1];
                // GLOG("Past last position keyframe: (%f,%f,%f)", pos.x, pos.y, pos.z);
            }
            else if (nextIndex == 0)
            {
                pos = animChannel->positions[0];
                // GLOG("Before first position keyframe: (%f,%f,%f)", pos.x, pos.y, pos.z);
            }
            else
            {
                const float startTime = animChannel->posTimeStamps[prevIndex];
                const float endTime   = animChannel->posTimeStamps[nextIndex];
                const float timeDiff  = endTime - startTime;

                float lambda          = (timeDiff > 0.0001f) ? (currentTime - startTime) / timeDiff : 0.0f;

                lambda                = (lambda < 0) ? 0 : (lambda > 1) ? 1 : lambda;

                pos = float3::Lerp(animChannel->positions[prevIndex], animChannel->positions[nextIndex], lambda);
                /*GLOG(
                    "Position interpolation: (%f,%f,%f), Lambda: %f, Times: %f to %f", pos.x, pos.y, pos.z, lambda,
                    startTime, endTime
                );*/
            }
        }
    }

    if (animChannel->numRotations > 0)
    {
        if (animChannel->numRotations == 1)
        {
            rot = animChannel->rotations[0].Normalized();
            GLOG("Single rotation keyframe: (%f,%f,%f,%f)", rot.x, rot.y, rot.z, rot.w);
        }
        else
        {
            size_t nextIndex = 0;
            while (nextIndex < animChannel->numRotations && animChannel->rotTimeStamps[nextIndex] <= currentTime)
            {
                nextIndex++;
            }

            size_t prevIndex = (nextIndex > 0) ? nextIndex - 1 : 0;

            if (nextIndex >= animChannel->numRotations)
            {
                rot = animChannel->rotations[animChannel->numRotations - 1].Normalized();
                // GLOG("Past last rotation keyframe: (%f,%f,%f,%f)", rot.x, rot.y, rot.z, rot.w);
            }
            else if (nextIndex == 0)
            {
                rot = animChannel->rotations[0].Normalized();
                // GLOG("Before first rotation keyframe: (%f,%f,%f,%f)", rot.x, rot.y, rot.z, rot.w);
            }
            else
            {
                const float startTime = animChannel->rotTimeStamps[prevIndex];
                const float endTime   = animChannel->rotTimeStamps[nextIndex];
                const float timeDiff  = endTime - startTime;

                float lambda          = (timeDiff > 0.0001f) ? (currentTime - startTime) / timeDiff : 0.0f;

                lambda                = (lambda < 0) ? 0 : (lambda > 1) ? 1 : lambda;

                rot = Interpolate(animChannel->rotations[prevIndex], animChannel->rotations[nextIndex], lambda);

                /*GLOG(
                    "Rotation interpolation: (%f,%f,%f,%f), Lambda: %f, Times: %f to %f", rot.x, rot.y, rot.z, rot.w,
                    lambda, startTime, endTime
                );*/
            }
        }
    }
}