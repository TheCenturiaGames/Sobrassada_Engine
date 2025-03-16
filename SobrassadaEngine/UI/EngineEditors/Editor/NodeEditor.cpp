#include "UI/EngineEditors/Editor/NodeEditor.h"
#include "UI/EngineEditors/Nodes/CustomNode.h"

NodeEditor::~NodeEditor()
{
}

bool NodeEditor::RenderEditor()
{
    if (!EngineEditorBase::RenderEditor()) return false;

    ImGui::Begin(name.c_str());

    myGrid->update();

    myGrid->rightClickPopUpContent(
        [this](ImFlow::BaseNode* node)
        {
            if (node == nullptr)
            {
                if (ImGui::MenuItem("Add Node"))
                {

                    ImVec2 mousePos = ImGui::GetMousePos();
                    ImVec2 gridPos  = myGrid->screen2grid(mousePos);
                    auto newNode    = myGrid->placeNodeAt<CustomNode>(gridPos);
                }
            }
        }
    );

    myGrid->droppedLinkPopUpContent(
        [this](ImFlow::Pin* dragged)
        {
            if (dragged && ImGui::MenuItem("Create and connect node"))
            {
                auto newNode = myGrid->placeNode<CustomNode>();
                if (newNode)
                {
                    auto customNode = std::dynamic_pointer_cast<CustomNode>(newNode);
                    if (customNode && customNode->getInputPin())
                    {
                        ImFlow::Pin* inputPinRaw = customNode->getInputPin().get();
                        if (inputPinRaw)
                        {

                            auto newLink = std::make_shared<ImFlow::Link>(dragged, inputPinRaw, myGrid.get());
                            myGrid->addLink(newLink);
                        }
                    }
                }
            }
        }
    );
    ImVec2 currentSize = ImGui::GetWindowSize();
    if (currentSize.x < 150 || currentSize.y < 150)
    {
        ImGui::SetWindowSize(ImVec2(150, 150), ImGuiCond_Always);
    }

    ImGui::End();
    return true;
}
