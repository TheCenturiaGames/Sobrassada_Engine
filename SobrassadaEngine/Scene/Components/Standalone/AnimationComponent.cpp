#include "AnimationComponent.h"

#include "AnimController.h"
#include "Application.h"
#include "CameraModule.h"
#include "EditorUIModule.h"
#include "GameObject.h"
#include "LibraryModule.h"
#include "Resource.h"
#include "ResourceAnimation.h"
#include "ResourceModel.h"
#include "ResourceStateMachine.h"
#include "ResourcesModule.h"
#include "SceneModule.h"
#include "StateMachineEditor.h"

#include "Math/Quat.h"
#include "imgui.h"

AnimationComponent::AnimationComponent(const UID uid, GameObject* parent)
    : Component(uid, parent, "Animation", COMPONENT_ANIMATION)
{
    animController = new AnimController();
}

AnimationComponent::AnimationComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent)
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

    if (initialState.HasMember("StateMachine") && initialState["StateMachine"].IsUint64())
    {
        UID smUID            = initialState["StateMachine"].GetUint64();
        resourceStateMachine = static_cast<ResourceStateMachine*>(App->GetResourcesModule()->RequestResource(smUID));
    }
    else
    {
        resourceStateMachine = nullptr;
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

void AnimationComponent::Init()
{
    currentAnimResource = static_cast<ResourceAnimation*>(App->GetResourcesModule()->RequestResource(resource));
    currentAnimName = App->GetLibraryModule()->GetResourceName(resource);
}

void AnimationComponent::OnPlay(bool isTransition)
{
    StateMachineEditor* stateMachine = nullptr;
    unsigned transitionTime          = 0;
    if (animController != nullptr && resource != INVALID_UID)
    {
        if (resourceStateMachine)
        {
            const State* activeState = resourceStateMachine->GetActiveState();
            for (const auto& state : resourceStateMachine->states)
            {
                if (state.name.GetString() == activeState->name.GetString())
                {
                    for (const auto& transition : resourceStateMachine->transitions)
                    {
                        if (state.name.GetString() == transition.toState.GetString())
                        {
                            transitionTime = transition.interpolationTime;
                        }
                    }
                    for (const auto& clip : resourceStateMachine->clips)
                    {
                        if (clip.clipName.GetString() == activeState->clipName.GetString())
                        {
                            GLOG("TransitionTime: %f", transitionTime);
                            if (isTransition)
                                animController->SetTargetAnimationResource(
                                    clip.animationResourceUID, transitionTime, clip.loop
                                );
                            else animController->Play(clip.animationResourceUID, clip.loop);
                            resource = clip.animationResourceUID;
                        }
                    }
                }
            }
        }
        else animController->Play(resource, true);
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
    std::string originAnimation = "";
    if (resource != 0)
    {
        const size_t underscorePos = currentAnimName.find('_');
        if (underscorePos != std::string::npos) originAnimation = currentAnimName.substr(0, underscorePos);
        if (currentAnimResource != nullptr)
        {
            ImGui::Text("Animation: %s", currentAnimName.c_str());
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

    if (ImGui::CollapsingHeader("Object Selection", ImGuiTreeNodeFlags_DefaultOpen))
    {

        GameObject* selectedObj = App->GetSceneModule()->GetScene()->GetSelectedGameObject();
        if (selectedObj && selectedObj == parent) currentAnimComp = this;

        if (selectedObj)
        {
            ImGui::Text("Selected Object: %s", selectedObj->GetName().c_str());

            currentAnimComp = static_cast<AnimationComponent*>(selectedObj->GetAnimationComponent());

            if (currentAnimComp)
            {
                ImGui::Text("Animation Component Found");

                // Get the current animation
                ResourceAnimation* anim = currentAnimComp->GetCurrentAnimation();
                if (anim)
                {
                    ImGui::Text("Current Animation: %s", anim->GetName().c_str());
                    animationDuration = anim->GetDuration();

                    // Display animation controls
                    ImGui::Separator();
                    ImGui::Text("Animation Controls");

                    if (ImGui::Button("Play"))
                    {
                        playing = true;
                        currentAnimComp->OnPlay(false);
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("Pause"))
                    {
                        playing = false;
                        currentAnimComp->OnPause();
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("Stop"))
                    {
                        playing     = false;
                        currentTime = 0.0f;
                        currentAnimComp->OnStop();
                    }

                    if (ImGui::Button("Resume"))
                    {
                        playing = true;
                        currentAnimComp->OnResume();
                    }

                    if (ImGui::SliderFloat("Timeline", &currentTime, 0.0f, animationDuration, "%.2f sec"))
                    {
                        // When user manually changes the time, update the animation controller
                        if (currentAnimComp->GetAnimationController())
                        {
                            currentAnimComp->GetAnimationController()->SetTime(currentTime);
                        }
                    }

                    // Bone visualization
                    if (ImGui::TreeNode("Bone Mapping"))
                    {
                        if (currentAnimComp)
                        {

                            const auto& boneMap = currentAnimComp->GetBoneMapping();

                            if (boneMap.empty())
                            {
                                ImGui::Text("No bones mapped. Animation might not apply correctly.");
                            }
                            else
                            {
                                ImGui::Text("%d bones mapped:", boneMap.size());
                                for (const auto& pair : boneMap)
                                {
                                    bool foundInAnimation = false;

                                    if (currentAnimComp->GetCurrentAnimation())
                                    {
                                        foundInAnimation =
                                            currentAnimComp->GetCurrentAnimation()->channels.find(pair.first) !=
                                            currentAnimComp->GetCurrentAnimation()->channels.end();
                                    }

                                    ImGui::TextColored(
                                        foundInAnimation ? ImVec4(0, 1, 0, 1) : ImVec4(1, 0, 0, 1), "%s -> %s",
                                        pair.first.c_str(), pair.second ? pair.second->GetName().c_str() : "NULL"
                                    );
                                }
                            }
                        }
                        ImGui::TreePop();
                    }
                }
                else
                {
                    ImGui::Text("No animation assigned to component");
                }
            }
            else
            {
                ImGui::Text("No animation component found on selected object");
            }
        }
        else
        {
            ImGui::Text("No object selected");
        }
    }

    if (ImGui::CollapsingHeader("Animation Library"))
    {
        std::string selectedZombunnyAnim = "";

        ImGui::Text("Available Animations:");
        const std::unordered_map<std::string, UID>& animationMap = App->GetLibraryModule()->GetAnimMap();

        for (const auto& pair : animationMap)
        {
            const std::string& animationName = pair.first;

            if (animationName.rfind(originAnimation, 0) == 0)
            {
                const bool isSelected = (selectedZombunnyAnim == animationName);

                if (ImGui::Selectable(animationName.c_str(), isSelected))
                {
                    selectedZombunnyAnim = animationName;
                    resource             = pair.second;

                    if (currentAnimComp->playing)
                    {
                        playing     = false;
                        currentTime = 0.0f;
                        currentAnimComp->OnStop();
                    }

                    GLOG("Selected animation: %s (UID: %llu)", animationName.c_str(), resource);
                }
            }
        }
    }

    ImGui::Separator();
    ImGui::Text("Associated State Machine");

    const std::unordered_map<std::string, UID>& stateMap = App->GetLibraryModule()->GetStateMachineMap();

    std::string currentName                              = "None";
    if (resourceStateMachine) currentName = resourceStateMachine->GetName();

    if (ImGui::BeginCombo("##StateMachineCombo", currentName.c_str()))
    {
        for (const auto& [name, uid] : stateMap)
        {
            const bool isSelected = (resourceStateMachine && resourceStateMachine->GetUID() == uid);
            if (ImGui::Selectable(name.c_str(), isSelected))
            {
                if (resourceStateMachine) App->GetResourcesModule()->ReleaseResource(resourceStateMachine);
                resourceStateMachine =
                    static_cast<ResourceStateMachine*>(App->GetResourcesModule()->RequestResource(uid));
            }
            if (isSelected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    if (resourceStateMachine)
    {
        ImGui::Separator();
        ImGui::Text("Available Triggers:");

        for (const std::string& triggerName : resourceStateMachine->availableTriggers)
        {
            if (ImGui::Button(triggerName.c_str()))
            {
                if (IsPlaying())
                {
                    GLOG("Trigger selected: %s", triggerName.c_str());
                    for (const auto& transition : resourceStateMachine->transitions)
                    {
                        if (transition.triggerName == triggerName &&
                            transition.fromState.GetString() ==
                                resourceStateMachine->GetActiveState()->name.GetString())
                        {
                            for (size_t i = 0; i < resourceStateMachine->states.size(); ++i)
                            {
                                if (resourceStateMachine->states[i].name.GetString() == transition.toState.GetString())
                                {
                                    resourceStateMachine->SetActiveState(static_cast<int>(i));
                                    OnPlay(true);
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (playing && currentAnimComp && currentAnimComp->GetAnimationController())
    {

        currentTime = currentAnimComp->GetAnimationController()->GetTime();

        if (currentTime >= animationDuration)
        {
            currentTime = 0.0f;
        }
    }
}

void AnimationComponent::Render(float deltaTime)
{
    if (!IsEffectivelyEnabled()) return;
}

void AnimationComponent::RenderDebug(float deltaTime)
{
}

void AnimationComponent::Clone(const Component* other)
{
    if (other->GetType() == ComponentType::COMPONENT_ANIMATION)
    {
        const AnimationComponent* otherAnimation = static_cast<const AnimationComponent*>(other);
        enabled                                  = otherAnimation->enabled;

        resource                                 = otherAnimation->resource;
        if (otherAnimation->currentAnimResource) AddAnimation(otherAnimation->currentAnimResource->GetUID());
        resourceStateMachine = otherAnimation->resourceStateMachine;
        animController       = otherAnimation->animController;
    }
    else
    {
        GLOG("It is not possible to clone a component of a different type!");
    }
}

void AnimationComponent::Update(float deltaTime)
{
    if (!IsEffectivelyEnabled()) return;
    if (!animController->IsPlaying()) return;

    if (boneMapping.empty())
    {
        SetBoneMapping();
    }

    animController->Update(deltaTime);

    for (auto& channel : currentAnimResource->channels)
    {
        const std::string& boneName = channel.first;

        auto boneIt                 = boneMapping.find(boneName);
        if (boneIt != boneMapping.end())
        {
            GameObject* bone       = boneIt->second;
            float4x4 boneTransform = bone->GetLocalTransform();

            float3 position        = boneTransform.TranslatePart();
            Quat rotation          = Quat(boneTransform.RotatePart());

            animController->GetTransform(boneName, position, rotation);

            float4x4 transformMatrix = float4x4::FromTRS(position, rotation, float3(1.0f, 1.0f, 1.0f));
            bone->SetLocalTransform(transformMatrix);
            bone->OnTransformUpdated();
        }
    }
}

void AnimationComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);

    targetState.AddMember(
        "Animations", currentAnimResource != nullptr ? currentAnimResource->GetUID() : INVALID_UID, allocator
    );
    targetState.AddMember(
        "StateMachine", resourceStateMachine != nullptr ? resourceStateMachine->GetUID() : INVALID_UID, allocator
    );
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
        currentAnimResource = newAnimation;
        currentAnimName     = currentAnimResource->GetName();
        resource            = animationUID;
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

bool AnimationComponent::IsPlaying() const
{
    return animController->IsPlaying();
}
