#include "AnimationComponent.h"
#include "ResourceManagement/Resources/ResourceAnimation.h"
#include "ResourceManagement/Resources/Resource.h"
#include "Application.h"
#include "CameraModule.h"
#include "EditorUIModule.h"
#include "LibraryModule.h"
#include "ResourcesModule.h"
#include "SceneModule.h"

#include "imgui.h"
#include <Math/Quat.h>

AnimationComponent::AnimationComponent(
    const UID uid, const UID uidParent
)
    : Component(uid, uidParent, "Animation", COMPONENT_ANIMATION)
{
    animController = new AnimController();
}

AnimationComponent::AnimationComponent(const rapidjson::Value& initialState) : Component(initialState)
{
    animController = new AnimController();
    if (initialState.HasMember("AnimResource") && initialState["AnimResource"].IsUint64())
    {
        resource = initialState["AnimResource"].GetUint64();
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
    }
}

void AnimationComponent::OnInspector()
{
    if (resource != 0)
    {
        currentAnim = dynamic_cast<ResourceAnimation*>(App->GetResourcesModule()->RequestResource(resource));
        
        if (currentAnim != nullptr)
        {
            ImGui::Text("Animation: %s", currentAnim->GetName().c_str());
            ImGui::Text("Duration: %.2f seconds", currentAnim->GetDuration());

            // Controls
            if (ImGui::Button("Play"))
            {
                OnPlay();
            }
            ImGui::SameLine();
            if (ImGui::Button("Stop"))
            {
                OnStop();
            }

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

    // Botón para seleccionar animación
    if (ImGui::Button("Select Animation"))
    {
        // Aquí podrías abrir un selector de recursos de animación
        // O simplemente usar el módulo de selección de recursos que ya tengas
    }
}

void AnimationComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);

    targetState.AddMember("Animation", currentAnim != nullptr ? currentAnim->GetUID() : INVALID_UID, allocator);
 
}

void AnimationComponent::AddAnimation(UID resource)
{
    if (currentAnim != nullptr && currentAnim->GetUID() == resource) return;

    ResourceAnimation* newAnimation=
        dynamic_cast<ResourceAnimation*>(App->GetResourcesModule()->RequestResource(resource));
    if (newAnimation != nullptr)
    {
        App->GetResourcesModule()->ReleaseResource(currentAnim);
        currentAnim     = newAnimation;
        
    }
}

void AnimationComponent::SetAnimationResource(UID animResource)
{
    resource = animResource;
}
