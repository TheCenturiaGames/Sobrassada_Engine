#include "AnimatorEditor.h"
#include "Standalone/AnimationComponent.h"
#include "Application.h"
#include "SceneModule.h"
#include "Scene/Scene.h"
#include "ResourceManagement/Resources/ResourceAnimation.h"
#include "imgui.h"

AnimatorEditor::AnimatorEditor(const std::string& editorName, const UID uid) : EngineEditorBase(editorName, uid)
{
    
    selectedObject    = nullptr;
    selectedAnimation = nullptr;
}

AnimatorEditor::~AnimatorEditor()
{
}

bool AnimatorEditor::RenderEditor()
{
   
    bool stillOpen                             = true;
   

    if (ImGui::Begin(name.c_str(), &stillOpen))
    {
       
        if (ImGui::CollapsingHeader("Object Selection", ImGuiTreeNodeFlags_DefaultOpen))
        {
            
            GameObject* selectedObj = App->GetSceneModule()->GetScene()->GetSelectedGameObject();

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
                            currentAnimComp->OnPlay();
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
                            playing     = true;
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
            
            ImGui::Text("Available Animations:");
           
        }

        
        if (playing && currentAnimComp && currentAnimComp->GetAnimationController())
        {
          
            currentTime = currentAnimComp->GetAnimationController()->GetTime();

            
            if (currentTime >= animationDuration)
            {
                currentTime = 0.0f;
            }
        }

        ImGui::End();
    }

    return stillOpen;
}


