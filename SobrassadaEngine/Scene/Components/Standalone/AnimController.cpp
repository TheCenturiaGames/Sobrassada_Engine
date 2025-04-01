#include "AnimController.h"
#include "ResourcesModule.h"
#include "EngineTimer.h"
#include "ResourceAnimation.h"
#include "Application.h"



AnimController::AnimController() : resource(0), currentTime(0), loop(false), playAnimation(false)
{
}

void AnimController::Play(UID newResource, bool shouldLoop)
{
    resource    = newResource;
    currentTime = 0;
    loop        = shouldLoop;
    playAnimation = true;
  
}

void AnimController::Stop()
{
    playAnimation = false;
    currentTime   = 0.0f;
}

void AnimController::Pause()
{
    playAnimation = false;
}

void AnimController::Resume()
{
    playAnimation = true;
}


update_status AnimController::Update()
{
   float deltaTime        = App->GetEngineTimer()->GetTime() * playbackSpeed;
    currentTime          += deltaTime;
    animation = static_cast<ResourceAnimation*>(App->GetResourcesModule()->RequestResource(resource));
   
    const float duration = animation->GetDuration();
    
    if (currentTime > duration)
    {
        if (loop)
        {
            
            currentTime = fmod(currentTime, duration);
        }
        else
        {
          
           Stop();
        }
    }
    App->GetResourcesModule()->ReleaseResource(animation);
    
    return UPDATE_CONTINUE;
}

void AnimController::GetTransform(const std::string& nodeName, float3& pos, Quat& rot)
{
    if (!playAnimation || resource == 0)
    {
        return;
    }

    animation            = static_cast<ResourceAnimation*>(App->GetResourcesModule()->RequestResource(resource));

    Channel* animChannel = animation->GetChannel(nodeName);

    GLOG("POSITIONS of channel %s: %d", nodeName.c_str(), animChannel->numPositions);

    if (animChannel->numPositions > 0 )
    {
        if (animChannel->numPositions == 1)
        {
            pos = animChannel->positions[0];
        }
        else
        {
            
            const auto upper =
                std::upper_bound(animChannel->posTimeStamps.begin(), animChannel->posTimeStamps.end(), currentTime);

            size_t index = std::distance(animChannel->posTimeStamps.begin(), upper);

            if (index == 0)
            {
                pos = animChannel->positions[0];
            }
            else if (index >= animChannel->numPositions)
            {
                pos = animChannel->positions.back();
            }
            else
            {
                float lambda = (currentTime - animChannel->posTimeStamps[index - 1]) /
                               (animChannel->posTimeStamps[index] - animChannel->posTimeStamps[index - 1]);

              
                pos = float3::Lerp(animChannel->positions[index - 1], animChannel->positions[index], lambda);

                GLOG("CURRENTTime: %f y position: (%f,%f,%f)", currentTime, pos.x, pos.y, pos.z);
            }
        }
    }

  
    if (!animChannel->rotations.empty() && animChannel->numRotations > 0)
    {
        if (animChannel->numRotations == 1)
        {
            rot = animChannel->rotations[0];
        }
        else
        {
           
            const auto upper =
                std::upper_bound(animChannel->rotTimeStamps.begin(), animChannel->rotTimeStamps.end(), currentTime);

            size_t index = std::distance(animChannel->rotTimeStamps.begin(), upper);

            if (index == 0)
            {
                rot = animChannel->rotations[0].Normalized();
            }
            else if (index >= animChannel->numRotations)
            {
                rot = animChannel->rotations.back().Normalized();
            }
            else
            {
                float lambda = (currentTime - animChannel->rotTimeStamps[index - 1]) /
                               (animChannel->rotTimeStamps[index] - animChannel->rotTimeStamps[index - 1]);

                
                rot = Quat::Slerp(animChannel->rotations[index - 1], animChannel->rotations[index], lambda).Normalized();

                GLOG("CURRENTTime: %f y rotation: (%f,%f,%f,%f)", currentTime, rot.x, rot.y, rot.z, rot.w);
            }
        }
    }
}

