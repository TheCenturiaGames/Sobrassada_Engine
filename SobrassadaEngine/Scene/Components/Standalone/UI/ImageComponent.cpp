#include "ImageComponent.h"

#include "Application.h"
#include "CanvasComponent.h"
#include "GameObject.h"
#include "LibraryModule.h"
#include "ResourcesModule.h"
#include "Transform2DComponent.h"

#include "glew.h"

ImageComponent::ImageComponent(UID uid, GameObject* parent) : Component(uid, parent, "Image", COMPONENT_IMAGE)
{
}

ImageComponent::ImageComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent)
{
}

ImageComponent::~ImageComponent()
{
}

void ImageComponent::Init()
{
    Component* transform = parent->GetComponentByType(COMPONENT_TRANSFORM_2D);
    if (transform == nullptr)
    {
        parent->CreateComponent(COMPONENT_TRANSFORM_2D);
        transform2D = static_cast<Transform2DComponent*>(parent->GetComponentByType(COMPONENT_TRANSFORM_2D));
    }
    else
    {
        transform2D = static_cast<Transform2DComponent*>(transform);
    }

    parentCanvas = transform2D->GetParentCanvas();

    if (parentCanvas == nullptr) GLOG("[WARNING] Label has no parent canvas, it won't be rendered");

    InitBuffers();
}
void ImageComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
}
void ImageComponent::Clone(const Component* other)
{
}

void ImageComponent::RenderEditorInspector()
{
}

void ImageComponent::RenderUI() const
{
    //const float vertices[] = {xPos,         yPos + height, 0.0f,         0.0f, xPos, yPos,
    //                          0.0f,         1.0f,          xPos + width, yPos, 1.0f, 1.0f,
    //
    //                          xPos + width, yPos + height, 1.0f,         0.0f, xPos, yPos + height,
    //                          0.0f,         0.0f,          xPos + width, yPos, 1.0f, 1.0f};
    //
    //GLuint lower           = static_cast<GLuint>(character.bindlessUID & 0xFFFFFFFF);
    //GLuint higher          = static_cast<GLuint>(character.bindlessUID >> 32);
    glUniform2ui(4, lower, higher);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
}

void ImageComponent::InitBuffers()
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

    // Set default texture
    texture = static_cast<ResourceTexture*>(
        App->GetResourcesModule()->RequestResource(App->GetLibraryModule()->GetTextureMap().at("DefaultTexture"))
    );
    bindlessUID = glGetTextureHandleARB(texture->GetTextureID());
    glMakeTextureHandleResidentARB(bindlessUID);
}

void ImageComponent::ClearBuffers()
{
    if (vbo != 0) glDeleteBuffers(1, &vbo);
    if (vao != 0) glDeleteVertexArrays(1, &vao);

    glMakeTextureHandleNonResidentARB(bindlessUID);
    App->GetResourcesModule()->ReleaseResource(texture);
}

void ImageComponent::OnTextureChange()
{
    glMakeTextureHandleNonResidentARB(bindlessUID);

    // Change resource texture

    bindlessUID = glGetTextureHandleARB(texture->GetTextureID());
    glMakeTextureHandleResidentARB(bindlessUID);
}