#include "ImageComponent.h"

#include "Application.h"
#include "CanvasComponent.h"
#include "GameObject.h"
#include "LibraryModule.h"
#include "ResourcesModule.h"
#include "Transform2DComponent.h"
#include "OpenGLModule.h"

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
    ClearBuffers();
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

void ImageComponent::RenderUI(const float4x4& view, const float4x4 proj) const
{
    if (parentCanvas == nullptr) return;

    const float3 startPos =
        float3(transform2D->GetRenderingPosition(), 0) - parent->GetGlobalTransform().TranslatePart();
    glUniformMatrix4fv(0, 1, GL_TRUE, parent->GetGlobalTransform().ptr());
    glUniformMatrix4fv(1, 1, GL_TRUE, view.ptr());
    glUniformMatrix4fv(2, 1, GL_TRUE, proj.ptr());

    glBindVertexArray(vao);

    //glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    const float vertices[] = {
        startPos.x,
        startPos.y,
        0.0f,
        0.0f,
        startPos.x,
        startPos.y - transform2D->size.y,
        0.0f,
        1.0f,
        startPos.x + transform2D->size.x,
        startPos.y - transform2D->size.y,
        1.0f,
        1.0f,

        startPos.x + transform2D->size.x,
        startPos.y,
        1.0f,
        0.0f,
        startPos.x,
        startPos.y,
        0.0f,
        0.0f,
        startPos.x + transform2D->size.x,
        startPos.y - transform2D->size.y,
        1.0f,
        1.0f
    };

    const GLuint lower  = static_cast<GLuint>(bindlessUID & 0xFFFFFFFF);
    const GLuint higher = static_cast<GLuint>(bindlessUID >> 32);
    glUniform2ui(4, lower, higher);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    App->GetOpenGLModule()->DrawArrays(GL_TRIANGLES, 0, 6);

    glDisable(GL_BLEND);
    //glEnable(GL_DEPTH_TEST);
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