#include "AnimationComponent.h"
#include "ResourceManagement/Resources/ResourceAnimation.h"
#include "ResourceManagement/Resources/Resource.h"
#include "GameObject.h"
#include "Application.h"
#include "CameraModule.h"
#include "EditorUIModule.h"
#include "LibraryModule.h"
#include "ResourcesModule.h"
#include "SceneModule.h"

#include "imgui.h"
#include <Math/Quat.h>

AnimationComponent::AnimationComponent(
    const UID uid, GameObject* parent
)
    : Component(uid, parent, "Animation", COMPONENT_ANIMATION)
{
    animController = new AnimController();
  
}

AnimationComponent::AnimationComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent )
{
    animController = new AnimController();
    if (initialState.HasMember("Animations") && initialState["Animations"].IsUint64())
    {
        resource = initialState["Animations"].GetUint64();
    }
    else
    {
        resource = 0;
    }
}

AnimationComponent::~AnimationComponent()
{
    if (animController != nullptr)
    {
        delete animController;
        animController = nullptr;
    }
    App->GetResourcesModule()->ReleaseResource(currentAnimResource);
}

void AnimationComponent::OnPlay()
{
    if (animController != nullptr && resource != 0)
    {
        animController->Play(resource, true);
    }
}

void AnimationComponent::OnStop()
{
    if (animController != nullptr)
    {
        animController->Stop();
    }
}

void AnimationComponent::OnUpdate()
{
    if (animController != nullptr)
    {
        animController->Update();
        GameObject* ownerObj = parent;
        if (ownerObj != nullptr)
        {
          
            for (const UID childUID : ownerObj->GetChildren())
            {
                GameObject* childObj = App->GetSceneModule()->GetScene()->GetGameObjectByUID(childUID);
                if (childObj != nullptr)
                {
                  
                    std::string nodeName = childObj->GetName();

                   
                    float3 position;
                    Quat rotation;
                    animController->GetTransform(nodeName, position, rotation);

                   
                    float4x4 transformMatrix = float4x4::FromTRS(position, rotation, float3(1.0f, 1.0f, 1.0f));

                    
                    childObj->SetLocalTransform(transformMatrix);

                    
                    //childObj->OnTransformUpdated();
                }
            }
        }
    }
}

void AnimationComponent::OnInspector()
{
   
    if (resource != 0)
    {
        currentAnimResource = dynamic_cast<ResourceAnimation*>(App->GetResourcesModule()->RequestResource(resource));
        
        if (currentAnimResource != nullptr)
        {
            ImGui::Text("Animation: %s", currentAnimResource->GetName().c_str());
            ImGui::Text("Duration: %.2f seconds", currentAnimResource->GetDuration());

             //Animation Time
            if (animController != nullptr && ImGui::TreeNode("Channels"))
            {
                // Channel Information
                ImGui::TreePop();
            }
        }
        else
        {
            ImGui::Text("Animation resource not found");
        }
    }
    else
    {
        ImGui::Text("No animation assigned");
    }

 
    if (ImGui::Button("Select Animation"))
    {
        
    }
}

void AnimationComponent::Render(float deltaTime)
{
}

void AnimationComponent::Update(float deltaTime)
{
}

void AnimationComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);

    targetState.AddMember("Animations", currentAnimResource != nullptr ? currentAnimResource->GetUID() : INVALID_UID, allocator);
 
}

void AnimationComponent::AddAnimation(UID animationUID)
{
    if (animationUID == INVALID_UID) return;
    if (currentAnimResource != nullptr && currentAnimResource->GetUID() == animationUID) return;

    ResourceAnimation* newAnimation =
        dynamic_cast<ResourceAnimation*>(App->GetResourcesModule()->RequestResource(animationUID));

    if (newAnimation != nullptr)
    {
        App->GetResourcesModule()->ReleaseResource(currentAnimResource);
        currentAnimResource     = newAnimation;
        currentAnimName = currentAnimResource->GetName();
        resource        = animationUID;
    }
}

void AnimationComponent::SetAnimationResource(UID animResource)
{
    resource = animResource;
    AddAnimation(resource);
    GLOG("Setting animation resource: %llu", resource);
  
}

void AnimationComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();
    
        OnInspector();
}


