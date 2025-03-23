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

    graph->rightClickPopUpContent(
        [this](ImFlow::BaseNode* node)
        {
            if (node == nullptr && ImGui::MenuItem("Add State"))
            {
                ImVec2 pos = graph->screen2grid(ImGui::GetMousePos());
                State newState;
                newState.name     = HashString("NewState_" + std::to_string(resource->states.size()));
                newState.clipName = HashString(""); 

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
    return true;
}

void StateMachineEditor::BuildGraph()
{
    
}