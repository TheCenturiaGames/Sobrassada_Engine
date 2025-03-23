#include "UI/EngineEditors/Editor/StateMachineEditor.h"
#include "UI/EngineEditors/Nodes/StateNode.h"

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
                State newState;
                std::string stateName = "NewState_" + std::to_string(resource->states.size());
                std::string clipName  = "Clip_" + std::to_string(resource->clips.size());

                newState.name         = HashString(stateName);
                newState.clipName     = HashString(clipName);

                //StateNode* stateNode  = dynamic_cast<StateNode*>(node);
                //stateNode->SetStateName(stateName);
                //stateNode->SetClipName(clipName);

                resource->AddClip(0, clipName, false); 
                resource->AddState(newState.name.GetString(), newState.clipName.GetString());
                graph->placeNodeAt<StateNode>(pos);
            }
        }
    );
    
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

    if (selectedNode != nullptr)
    {
        ImGui::Begin("State Inspector");

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

        if (ImGui::InputText("Clip Name", clipBuffer, sizeof(clipBuffer)))
        {
            std::string newClip(clipBuffer);
            if (newClip != selectedNode->GetClipName())
            {
                resource->EditState(selectedNode->GetStateName(), selectedNode->GetStateName(), newClip);
                selectedNode->SetClipName(newClip); 
            }
        }

        const Clip* clip = resource->GetClip(selectedNode->GetClipName());
        if (clip)
        {
            static UID clipUIDBuffer = clip->clipUID;
            static bool loopBuffer   = clip->loop;

            ImGui::InputScalar("Clip UID", ImGuiDataType_U64, &clipUIDBuffer);
            ImGui::Checkbox("Loop", &loopBuffer);

            char newClipNameBuffer[128];
            strncpy_s(newClipNameBuffer, clip->clipName.GetString().c_str(), sizeof(newClipNameBuffer));
            newClipNameBuffer[sizeof(newClipNameBuffer) - 1] = '\0';

            if (ImGui::InputText("Rename Clip", newClipNameBuffer, sizeof(newClipNameBuffer)))
            {
                std::string newClipName(newClipNameBuffer);

                if (newClipName != clip->clipName.GetString() || clipUIDBuffer != clip->clipUID ||
                    loopBuffer != clip->loop)
                {
                    resource->EditClipInfo(clip->clipName.GetString(), clipUIDBuffer, newClipName, loopBuffer);
                }
            }
        }
        else
        {
            ImGui::TextColored(
                ImVec4(1, 0.5f, 0.5f, 1.0f), "No Clip found with name: %s", selectedNode->GetClipName().c_str()
            );
        }

        ImGui::End();
    }

    return true;
}

void StateMachineEditor::BuildGraph()
{
    
}