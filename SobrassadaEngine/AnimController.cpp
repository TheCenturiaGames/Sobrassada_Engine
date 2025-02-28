#include "AnimController.h"
#include "ResourcesModule.h"
#include "Application.h"

AnimController::AnimController() : resource(0), currentTime(0), loop(false)
{
}

void AnimController::SetAnimation(ResourceAnimation* animation)
{
    if (!animation) return;

    resource    = animation->GetUID();
    currentTime = 0;
}

void AnimController::Play(UID newResource, bool shouldLoop)
{
    resource    = newResource;
    currentTime = 0;
    loop        = shouldLoop;
}

void AnimController::Stop()
{
    resource    = 0;
    currentTime = 0;
    loop        = false;
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
            currentTime = duration; 
        }
    }

    return UPDATE_CONTINUE;
}

void AnimController::GetTransform(const std::string& nodeName, float3& pos, Quat& rot)
{

}