#include "UILabelComponent.h"

#include "Application.h"
#include "CameraModule.h"
#include "ShaderModule.h"
#include "TextManager.h"
#include "WindowModule.h"

#include "glew.h"
#include "imgui.h"

UILabelComponent::UILabelComponent(UID uid, GameObject* parent)
    : text("XD"), Component(uid, parent, "Label", COMPONENT_LABEL)
{
    fontData = new TextManager::FontData();
    fontData->Init("./EngineDefaults/Shader/Font/Arial.ttf", fontSize);
    InitBuffers();

    // TODO: Get a reference to the parent canvas. Al instanciar un ui widget afegirli una referencia del canvas pare. Si no té dona igual perque no es fara render
    // Es mirarà el canvas pare per veure si es renderitza el text en world space o en screen space. Potser es pot cridar des del canvas una funció que no sigui render() 
    // als fills, i se li passi un bool de world o screen (de fet segurament es millor ja que a priori em sembla que també es soluciona el problema de la recursivitat)
}

UILabelComponent::~UILabelComponent()
{
    fontData->Clean();
    if (vbo != 0) glDeleteBuffers(1, &vbo);
    if (vao != 0) glDeleteBuffers(1, &vao);
}

void UILabelComponent::Clone(const Component* otherComponent)
{
}

void UILabelComponent::Update(float deltaTime)
{
}

void UILabelComponent::Render(float deltaTime)
{
    int uiProgram = App->GetShaderModule()->GetUIWidgetProgram();
    if (uiProgram == -1) GLOG("Error with UI Program");

    glUseProgram(uiProgram);

    float4x4 proj = float4x4::D3DOrthoProjLH(
        -1, 1, App->GetWindowModule()->GetWidth(), App->GetWindowModule()->GetHeight()
    ); // near plane. far plane, screen width, screen height

    glUniformMatrix4fv(0, 1, GL_TRUE, parent->GetGlobalTransform().ptr());
    //glUniformMatrix4fv(1, 1, GL_TRUE, App->GetCameraModule()->GetViewMatrix().ptr());
    //glUniformMatrix4fv(2, 1, GL_TRUE, App->GetCameraModule()->GetProjectionMatrix().ptr());

    glUniformMatrix4fv(1, 1, GL_TRUE, float4x4::identity.ptr());
    glUniformMatrix4fv(2, 1, GL_TRUE, proj.ptr());

    glBindVertexArray(vao);
    TextManager::RenderText(*fontData, text, vbo);
}

void UILabelComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();

    if (enabled)
    {
        ImGui::Text("Label");
        ImGui::InputText("Label Text", &text[0], text.size(), ImGuiInputTextFlags_ReadOnly);

        if (ImGui::InputInt("Font Size", &fontSize))
        {
            if (fontSize < 0) fontSize = 0;
        }
        ImGui::ColorPicker3("Font Color", &fontColor[0]);
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