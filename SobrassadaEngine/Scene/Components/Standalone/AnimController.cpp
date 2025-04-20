#include "AnimController.h"

#include "Application.h"
#include "EngineTimer.h"
#include "ResourceAnimation.h"
#include "ResourcesModule.h"

AnimController::AnimController()
    : resource(0), currentTime(0), loop(false), playAnimation(false), playbackSpeed(1.0f), currentAnimation(nullptr)
{
}

AnimController::~AnimController()
{
    App->GetResourcesModule()->ReleaseResource(currentAnimation);
}

void AnimController::Play(UID newResource, bool shouldLoop)
{
    if (currentAnimation == nullptr) Stop();
    resource         = newResource;
    currentTime      = 0.0f;
    loop             = shouldLoop;
    currentAnimation = static_cast<ResourceAnimation*>(App->GetResourcesModule()->RequestResource(resource));
    playAnimation    = true;
    playAnimation    = true;
    animationFinished = false;
}

void AnimController::Stop()
{
    playAnimation = false;
    currentTime   = 0.0f;

    if (currentAnimation != nullptr)
    {
        App->GetResourcesModule()->ReleaseResource(currentAnimation);
        currentAnimation = nullptr;
    }
}

Quat AnimController::Interpolate(Quat& first, Quat& second, float lambda)
{
    if (first.Dot(second) >= 0.0f) return Quat::Lerp(first, second, lambda).Normalized();
    else return Quat::Lerp(first, second.Neg(), lambda).Normalized();
}

update_status AnimController::Update(float deltaTime)
{
    if (!playAnimation || resource == INVALID_UID) return UPDATE_CONTINUE;

    deltaTime          *= playbackSpeed;

    /*GLOG(
        "Raw delta time: %f, Capped delta: %f, Playback speed: %f", App->GetEngineTimer()->GetTime(), rawDeltaTime,
        playbackSpeed
    );*/

    float previousTime  = currentTime;
    currentTime        += deltaTime;

    if (currentAnimation == nullptr) return UPDATE_CONTINUE;

    const float duration = currentAnimation->GetDuration();
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
            animationFinished = true;

            //if (currentAnimation != nullptr)
            //{
            //    App->GetResourcesModule()->ReleaseResource(currentAnimation);
            //    currentAnimation = nullptr;
            //}
        }
    }

    if (targetAnimation != nullptr)
    {
        currentTargetTime          += deltaTime;
        const float targetDuration  = targetAnimation->GetDuration();
        if (currentTargetTime > targetDuration) currentTargetTime = fmod(currentTargetTime, targetDuration);

        fadeTime += deltaTime;

        if (fadeTime >= transitionTime)
        {
            App->GetResourcesModule()->ReleaseResource(currentAnimation);
            currentAnimation  = targetAnimation;
            targetAnimation   = nullptr;
            currentTime       = currentTargetTime;
            fadeTime          = 0;
            currentTargetTime = 0;
        }
    }

    return UPDATE_CONTINUE;
}

void AnimController::GetTransform(const std::string& nodeName, float3& pos, Quat& rot)
{
    if (!playAnimation || resource == INVALID_UID || currentAnimation == nullptr) return;

    if (targetAnimation == nullptr)
    {
        Channel* animChannel = currentAnimation->GetChannel(nodeName);
        if (animChannel == nullptr) return;

        GetChannelPosition(animChannel, pos, currentTime);
        GetChannelRotation(animChannel, rot, currentTime);
    }
    else
    {
        float weight               = transitionTime != 0 ? fadeTime / transitionTime : 1;
        // GLOG("transitionTime: %f, fadeTime: %f, weight: %f", transitionTime, fadeTime, weight);
        Channel* animChannel       = currentAnimation->GetChannel(nodeName);
        Channel* targetAnimChannel = targetAnimation->GetChannel(nodeName);
        if (animChannel == nullptr || targetAnimChannel == nullptr) return;

        float3 animPos = float3(pos);
        Quat animQuat  = Quat(rot);
        GetChannelPosition(animChannel, animPos, currentTime);
        GetChannelRotation(animChannel, animQuat, currentTime);

        float3 targetAnimPos = float3(pos);
        Quat targetAnimQuat  = Quat(rot);
        GetChannelPosition(targetAnimChannel, targetAnimPos, currentTargetTime);
        GetChannelRotation(targetAnimChannel, targetAnimQuat, currentTargetTime);

        pos = animPos.Lerp(targetAnimPos, weight);
        rot = Quat::Slerp(animQuat, targetAnimQuat, weight);
    }

    // TODO Implement piecewise interpolation to support lerp between more than two animations
    // TODO
    // https://stackoverflow.com/questions/66522629/given-3-or-more-numbers-or-vectors-how-do-i-interpolate-between-them-based-on-a
}

void AnimController::SetTargetAnimationResource(UID uid, unsigned timeTransition, bool shouldLoop)
{
    targetAnimation = static_cast<ResourceAnimation*>(App->GetResourcesModule()->RequestResource(uid));
    transitionTime  = static_cast<float>(timeTransition) / 1000;
    loop            = shouldLoop;
    playAnimation     = true;
    animationFinished = false;
}

void AnimController::GetChannelPosition(const Channel* animChannel, float3& pos, const float time) const
{
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
            while (nextIndex < animChannel->numPositions && animChannel->posTimeStamps[nextIndex] <= time)
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

                float lambda          = (timeDiff > 0.0001f) ? (time - startTime) / timeDiff : 0.0f;

                lambda                = (lambda < 0) ? 0 : (lambda > 1) ? 1 : lambda;

                pos = float3::Lerp(animChannel->positions[prevIndex], animChannel->positions[nextIndex], lambda);
                /*GLOG(
                    "Position interpolation: (%f,%f,%f), Lambda: %f, Times: %f to %f", pos.x, pos.y, pos.z, lambda,
                    startTime, endTime
                );*/
            }
        }
    }
}

void AnimController::GetChannelRotation(Channel* animChannel, Quat& rot, const float time)
{
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
            while (nextIndex < animChannel->numRotations && animChannel->rotTimeStamps[nextIndex] <= time)
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

                float lambda          = (timeDiff > 0.0001f) ? (time - startTime) / timeDiff : 0.0f;

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
