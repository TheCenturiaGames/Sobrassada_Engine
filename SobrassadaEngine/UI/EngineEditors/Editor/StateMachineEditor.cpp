#include "UI/EngineEditors/Editor/StateMachineEditor.h"
#include "UI/EngineEditors/Nodes/StateNode.h"
#include "FileSystem/StateMachineManager.h"
#include "Application.h"
#include "ResourcesModule.h"

StateMachineEditor::StateMachineEditor(const std::string& editorName, UID uid, ResourceStateMachine* stateMachine)
    : EngineEditorBase(editorName, uid), uid(uid), resource(stateMachine)
{
    graph = std::make_unique<ImFlow::ImNodeFlow>("StateMachineGraph_" + std::to_string(uid));
    BuildGraph();
}

StateMachineEditor::~StateMachineEditor()
{
}

bool StateMachineEditor::RenderEditor()
{
    if (!EngineEditorBase::RenderEditor()) return false;

    ImGui::Begin(name.c_str());

     if (ImGui::Button("Save"))
    {
        SaveMachine();
    }
    ImGui::SameLine();
    if (ImGui::Button("Load"))
    {
        LoadMachine();
    }

    graph->update();

    for (auto& pair : graph->getNodes())
    {
        auto& node = pair.second;
        if (node->isSelected())
        {
            selectedNode = dynamic_cast<StateNode*>(node.get());
        }
    }
   

    graph->rightClickPopUpContent(
        [this](ImFlow::BaseNode* node)
        {
            if (node == nullptr && ImGui::MenuItem("Add State"))
            {
                ImVec2 pos = graph->screen2grid(ImGui::GetMousePos());
            
                auto newNode = graph->placeNodeAt<StateNode>(pos);
                if (newNode)
                {
                    auto newStateNode = std::dynamic_pointer_cast<StateNode>(newNode);
                    if (newStateNode)
                    {
                        CreateBaseState(*newStateNode.get());
                    }
                }
            }
        }
    );

    DetectNewTransitions();
    
    graph->droppedLinkPopUpContent(
        [this](ImFlow::Pin* dragged)
        {
            if (dragged && ImGui::MenuItem("Create Transition"))
            {
                auto sourceNode = dynamic_cast<StateNode*>(dragged->getParent());
                if (!sourceNode) return;

                auto newNode = graph->placeNode<StateNode>();
                if (newNode)
                {
                    auto inputPinRaw = std::dynamic_pointer_cast<StateNode>(newNode);
                    if (inputPinRaw)
                    {
                        CreateBaseState(*inputPinRaw.get());
                        resource->AddTransition(
                            sourceNode->GetStateName(), inputPinRaw->GetStateName(), "Trigger", 200
                        );
                        auto newLink =
                            std::make_shared<ImFlow::Link>(dragged, inputPinRaw->getInputPin().get(), graph.get());
                        graph->addLink(newLink);
                    }
                }
            }
        }
    );

    ImGui::End();

    //INSPECTOR

    if (selectedNode != nullptr)
    {
        ImGui::Begin("State Inspector");

        ImGui::Text("State");
        ImGui::Separator();

        std::string stateName = selectedNode->GetStateName();
        char nameBuffer[128];
        strncpy_s(nameBuffer, stateName.c_str(), sizeof(nameBuffer));
        nameBuffer[sizeof(nameBuffer) - 1] = '\0';

        if (ImGui::InputText("State Name", nameBuffer, sizeof(nameBuffer)))
        {
            std::string newName(nameBuffer);
            if (newName != stateName)
            {
                resource->EditState(stateName, newName, selectedNode->GetClipName());
                selectedNode->SetStateName(newName); 
            }
        }
        char clipBuffer[128];
        strncpy_s(clipBuffer, selectedNode->GetClipName().c_str(), sizeof(clipBuffer));
        clipBuffer[sizeof(clipBuffer) - 1] = '\0';

        if (ImGui::InputText("Associated Clip", clipBuffer, sizeof(clipBuffer)))
        {
            std::string newClip(clipBuffer);
            if (newClip != selectedNode->GetClipName())
            {
                resource->EditState(selectedNode->GetStateName(), selectedNode->GetStateName(), newClip);
                selectedNode->SetClipName(newClip); 
            }
        }

        
        ImGui::Spacing();
        ImGui::Text("Clip Information");
        ImGui::Separator();
        const Clip* clip = resource->GetClip(selectedNode->GetClipName());
        if (clip)
        {
            static bool loopBuffer = clip->loop;

            ImGui::Text("Clip UID: %llu", clip->clipUID);

            ImGui::Text("Clip Name: %s", clip->clipName.GetString().c_str());

            if (ImGui::Checkbox("Loop", &loopBuffer))
            {
                if (loopBuffer != clip->loop)
                {
                    resource->EditClipInfo(
                        clip->clipName.GetString(), clip->clipUID, clip->clipName.GetString(), loopBuffer
                    );
                }
            }
        }
        else
        {
            ImGui::TextColored(
                ImVec4(1, 0.5f, 0.5f, 1.0f), "No Clip found with name: %s", selectedNode->GetClipName().c_str()
            );
        }


        ImGui::Spacing();
        ImGui::Text("Connected Transitions");
        ImGui::Separator();
        int cont = 0;
        for (const auto& transition : resource->transitions)
        {
            if (transition.fromState.GetString() == selectedNode->GetStateName())
            {
                ImGui::Text(
                    "-> %s (Trigger: %s, Blend: %u ms)", transition.toState.GetString().c_str(),
                    transition.triggerName.GetString().c_str(), transition.interpolationTime
                );
            }
            else if (transition.toState.GetString() == selectedNode->GetStateName())
            {
                ImGui::Text(
                    "<- %s (Trigger: %s, Blend: %u ms)", transition.fromState.GetString().c_str(),
                    transition.triggerName.GetString().c_str(), transition.interpolationTime
                );
            }
        }

        ImGui::End();
    }

    return true;
}

void StateMachineEditor::BuildGraph()
{
    
}

void StateMachineEditor::DetectNewTransitions()
{
    for (const auto& linkWeak : graph->getLinks())
    {
        if (auto link = linkWeak.lock())
        {
            ImFlow::Pin* startPin = link->left(); 
            ImFlow::Pin* endPin   = link->right();

            if (!startPin || !endPin) continue;

            auto startNode = dynamic_cast<StateNode*>(startPin->getParent());
            auto endNode   = dynamic_cast<StateNode*>(endPin->getParent());

            if (startNode && endNode)
            {
                std::string fromState = startNode->GetStateName();
                std::string toState   = endNode->GetStateName();

                if (!resource->GetTransition(fromState, toState))
                {
                    GLOG("Creating transition from %s to %s", fromState.c_str(), toState.c_str());
                    resource->AddTransition(fromState, toState, "Trigger", 200);
                }
            }
        }
    }
}

void StateMachineEditor::CreateBaseState(StateNode& node)
{
    State newState;
    std::string stateName = "NewState_" + std::to_string(resource->states.size());
    std::string clipName  = "Clip_" + std::to_string(resource->clips.size());

    newState.name         = HashString(stateName);
    newState.clipName     = HashString(clipName);

    resource->AddClip(0, clipName, false);
    resource->AddState(newState.name.GetString(), newState.clipName.GetString());
    node.SetStateName(stateName);
    node.SetClipName(clipName);

}

void StateMachineEditor::SaveMachine()
{
    StateMachineManager::Save(resource, false);
}

void StateMachineEditor::LoadMachine()
{
    UID stateMachineUid = 1789842735177183;
    const ResourceStateMachine* stateMachine = (const ResourceStateMachine*)App->GetResourcesModule()->RequestResource(stateMachineUid);
}
