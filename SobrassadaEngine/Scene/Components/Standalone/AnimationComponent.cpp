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

void AnimationComponent::OnPause()
{
    if (animController != nullptr)
    {
        animController->Pause();
    }
}

void AnimationComponent::OnResume()
{
    if (animController != nullptr)
    {
        animController->Resume();
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

            
            if (animController != nullptr && ImGui::TreeNode("Channels"))
            {
                for (const auto& channel : currentAnimResource->channels)
                {
                    ImGui::Text("Bone: %s", channel.first.c_str());

                    if (ImGui::TreeNode(channel.first.c_str()))
                    {
                        const Channel& ch = channel.second;

                        ImGui::Text("Positions: %d", ch.numPositions);
                        if (ch.numPositions > 0)
                        {
                            ImGui::Text("First Position Time: %.2f", ch.posTimeStamps.front());
                            ImGui::Text("Last Position Time: %.2f", ch.posTimeStamps.back());
                        }

                        ImGui::Text("Rotations: %d", ch.numRotations);
                        if (ch.numRotations > 0)
                        {
                            ImGui::Text("First Rotation Time: %.2f", ch.rotTimeStamps.front());
                            ImGui::Text("Last Rotation Time: %.2f", ch.rotTimeStamps.back());
                        }

                        ImGui::TreePop();
                    }
                }
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
}

void AnimationComponent::Render(float deltaTime)
{
}

void AnimationComponent::Clone(const Component* other)
{
    if (other->GetType() == ComponentType::COMPONENT_ANIMATION)
    {
        const AnimationComponent* otherAnimation = static_cast<const AnimationComponent*>(other);
      
        AddAnimation(otherAnimation->currentAnimResource->GetUID());
        
    }
    else
    {
        GLOG("It is not possible to clone a component of a different type!");
    }
}

void AnimationComponent::Update(float deltaTime)
{
    if (!animController->IsPlaying()) return;
    
        if (boneMapping.empty())
        {
            SetBoneMapping();
        }

        animController->Update();

    for (auto& channel : currentAnimResource->channels)
    {
        const std::string& boneName = channel.first;

         auto boneIt                 = boneMapping.find(boneName);

         if (boneIt != boneMapping.end())
         {
             GameObject* bone = boneIt->second;
             float3 position;
             Quat rotation;

             animController->GetTransform(boneName, position, rotation);

             float4x4 transformMatrix = float4x4::FromTRS(position, rotation, float3(1.0, 1.0, 1.0));
             bone->SetLocalTransform(transformMatrix);
             bone->OnTransformUpdated();
         }
     }
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
        SetBoneMapping();
    }
}

void AnimationComponent::SetAnimationResource(UID animResource)
{
    resource = animResource;
    AddAnimation(resource);
    GLOG("Setting animation resource: %llu", resource);
  
}

void AnimationComponent::SetBoneMapping()
{
    boneMapping.clear();

    std::function<void(GameObject*)> mapBones = [this, &mapBones](GameObject* obj)
    {
        if (obj == nullptr) return;

        boneMapping[obj->GetName()] = obj;

        for (const UID childUID : obj->GetChildren())
        {
            GameObject* child = App->GetSceneModule()->GetScene()->GetGameObjectByUID(childUID);

            if (child != nullptr)
            {
                mapBones(child);
            }
        }
    };
    mapBones(parent);
}

void AnimationComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();
    if (enabled)
    {
        OnInspector();
    }
        
}


