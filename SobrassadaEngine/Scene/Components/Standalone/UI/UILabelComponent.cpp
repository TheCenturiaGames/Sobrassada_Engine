#include "UILabelComponent.h"

#include "Application.h"
#include "CameraModule.h"
#include "CanvasComponent.h"
#include "Scene.h"
#include "SceneModule.h"
#include "ShaderModule.h"
#include "TextManager.h"
#include "Transform2DComponent.h"
#include "WindowModule.h"

#include "glew.h"
#include "imgui.h"
#include <queue>

UILabelComponent::UILabelComponent(UID uid, GameObject* parent)
    : text("XD"), Component(uid, parent, "Label", COMPONENT_LABEL)
{
    Transform2DComponent* transform =
        static_cast<Transform2DComponent*>(parent->GetComponentByType(COMPONENT_TRANSFORM_2D));
    if (transform == nullptr)
    {
        parent->CreateComponent(COMPONENT_TRANSFORM_2D);
        transform2D = static_cast<Transform2DComponent*>(parent->GetComponentByType(COMPONENT_TRANSFORM_2D));
        transform2D->SetSize(App->GetWindowModule()->GetWidth(), App->GetWindowModule()->GetHeight());
    }
    else
    {
        transform2D = transform;
    }

    // Search for a parent canvas iteratively
    std::queue<UID> parentQueue;
    parentQueue.push(parent->GetParent());

    Scene* scene =
        App->GetSceneModule()->GetScene(); // Save the scene here to not call for it in each iteration of the loops
    while (!parentQueue.empty())
    {
        const GameObject* currentParent = scene->GetGameObjectByUID(parentQueue.front());

        if (currentParent == nullptr) break; // If parent null it has reached the scene root

        CanvasComponent* canvas = static_cast<CanvasComponent*>(currentParent->GetComponentByType(COMPONENT_CANVAS));
        if (canvas != nullptr)
        {
            parentCanvas = canvas;
            break;
        }

        parentQueue.push(currentParent->GetParent());
        parentQueue.pop();
    }

    fontData = new TextManager::FontData();
    fontData->Init("./EngineDefaults/Shader/Font/Arial.ttf", fontSize);
    InitBuffers();
}

UILabelComponent::~UILabelComponent()
{
    fontData->Clean();
    if (vbo != 0) glDeleteBuffers(1, &vbo);
    if (vao != 0) glDeleteVertexArrays(1, &vao);
}

void UILabelComponent::Clone(const Component* otherComponent)
{
    // It will have to look for the canvas here probably, seems better to do that in a separate function
}

void UILabelComponent::Update(float deltaTime)
{
}

void UILabelComponent::Render(float deltaTime)
{
    int uiProgram = App->GetShaderModule()->GetUIWidgetProgram();
    if (uiProgram == -1) GLOG("Error with UI Program");

    glUseProgram(uiProgram);

    const float4x4 view =
        parentCanvas->IsInWorldSpaceEditor() ? App->GetCameraModule()->GetViewMatrix() : float4x4::identity;
    const float4x4 proj = parentCanvas->IsInWorldSpaceEditor()
                            ? App->GetCameraModule()->GetProjectionMatrix()
                            : float4x4::D3DOrthoProjLH(
                                  -1, 1, App->GetWindowModule()->GetWidth(), App->GetWindowModule()->GetHeight()
                              ); // near plane. far plane, screen width, screen height

    glUniformMatrix4fv(0, 1, GL_TRUE, parent->GetGlobalTransform().ptr());
    glUniformMatrix4fv(1, 1, GL_TRUE, view.ptr());
    glUniformMatrix4fv(2, 1, GL_TRUE, proj.ptr());

    glUniform3fv(3, 1, fontColor.ptr()); // Font color

    glBindVertexArray(vao);
    TextManager::RenderText(*fontData, text, vbo);
}

void UILabelComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();

    if (enabled)
    {
        ImGui::Text("Label");
        ImGui::InputTextMultiline("Label Text", &text[0], sizeof(text));

        if (ImGui::InputInt("Font Size", &fontSize))
        {
            if (fontSize < 0) fontSize = 0;
        }
        ImGui::ColorPicker3("Font Color", fontColor.ptr());
    }
}

void UILabelComponent::InitBuffers()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}