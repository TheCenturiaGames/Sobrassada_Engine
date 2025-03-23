#include "UILabelComponent.h"

#include "TextManager.h"
#include "ShaderModule.h"
#include "Application.h"
#include "CameraModule.h"

#include "glew.h"

UILabelComponent::UILabelComponent(UID uid, GameObject* parent)
    : text("New text"), UIWidgetComponent(uid, parent, "Label", COMPONENT_LABEL)
{
    fontData = new TextManager::FontData();
    fontData->Init("./EngineDefaults/Shader/Font/Arial.ttf", fontSize);
}

UILabelComponent::~UILabelComponent()
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

    glUniformMatrix4fv(0, 1, GL_TRUE, parent->GetGlobalTransform().ptr());
    glUniformMatrix4fv(1, 1, GL_TRUE, App->GetCameraModule()->GetProjectionMatrix().ptr());
    glUniformMatrix4fv(2, 1, GL_TRUE, App->GetCameraModule()->GetViewMatrix().ptr());

    glBindVertexArray(vao);
    TextManager::RenderText(*fontData, text, vbo);
}

void UILabelComponent::Clone(const Component* otherComponent)
{
}

void UILabelComponent::InitBuffers()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, NULL, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}