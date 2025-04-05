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

UILabelComponent::UILabelComponent(UID uid, GameObject* parent)
    : text("Le Sobrassada"), Component(uid, parent, "Label", COMPONENT_LABEL)
{
    fontData = new TextManager::FontData();
}

UILabelComponent::UILabelComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent)
{
    fontData            = new TextManager::FontData();

    const char* textPtr = initialState["Text"].GetString();
    strcpy_s(text, sizeof(text), textPtr);
    fontSize = initialState["FontSize"].GetInt();

    if (initialState.HasMember("FontColor") && initialState["FontColor"].IsArray())
    {
        const rapidjson::Value& initFontColor = initialState["FontColor"];
        fontColor.x                           = initFontColor[0].GetFloat();
        fontColor.y                           = initFontColor[1].GetFloat();
        fontColor.z                           = initFontColor[2].GetFloat();
    }
}

UILabelComponent::~UILabelComponent()
{
    fontData->Clean();
    delete fontData;
    if (vbo != 0) glDeleteBuffers(1, &vbo);
    if (vao != 0) glDeleteVertexArrays(1, &vao);
}

void UILabelComponent::Init()
{
    Transform2DComponent* transform =
        static_cast<Transform2DComponent*>(parent->GetComponentByType(COMPONENT_TRANSFORM_2D));
    if (transform == nullptr)
    {
        parent->CreateComponent(COMPONENT_TRANSFORM_2D);
        transform2D = static_cast<Transform2DComponent*>(parent->GetComponentByType(COMPONENT_TRANSFORM_2D));
    }
    else
    {
        transform2D = transform;
    }

    parentCanvas = transform2D->GetParentCanvas();

    if (parentCanvas == nullptr) GLOG("[WARNING] Label has no parent canvas, it won't be rendered");

    fontData->Init("./EngineDefaults/Shader/Font/Arial.ttf", fontSize);
    InitBuffers();
}

void UILabelComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);

    std::string textString(text);
    targetState.AddMember("Text", rapidjson::Value(textString.c_str(), allocator), allocator);
    targetState.AddMember("FontSize", fontSize, allocator);

    rapidjson::Value valFontColor(rapidjson::kArrayType);
    valFontColor.PushBack(fontColor.x, allocator);
    valFontColor.PushBack(fontColor.y, allocator);
    valFontColor.PushBack(fontColor.z, allocator);
    targetState.AddMember("FontColor", valFontColor, allocator);
}

void UILabelComponent::Clone(const Component* other)
{
    // It will have to look for the canvas here probably, seems better to do that in a separate function
    if (other->GetType() == ComponentType::COMPONENT_LABEL)
    {
        const UILabelComponent* otherLabel = static_cast<const UILabelComponent*>(other);
        strcpy_s(text, sizeof(text), otherLabel->text);
        fontSize  = otherLabel->fontSize;
        fontColor = otherLabel->fontColor;

        fontData->Clean();
        fontData->Init("./EngineDefaults/Shader/Font/Arial.ttf", fontSize);
    }
    else
    {
        GLOG("It is not possible to clone a component of a different type!");
    }
}

void UILabelComponent::Update(float deltaTime)
{
}

void UILabelComponent::Render(float deltaTime)
{
    if (parentCanvas == nullptr) return;

    const int uiProgram = App->GetShaderModule()->GetUIWidgetProgram();
    if (uiProgram == -1)
    {
        GLOG("Error with UI Program");
        return;
    }

    glUseProgram(uiProgram);

    const float4x4& view =
        parentCanvas->IsInWorldSpaceEditor() ? App->GetCameraModule()->GetViewMatrix() : float4x4::identity;
    const float4x4& proj =
        parentCanvas->IsInWorldSpaceEditor()
            ? App->GetCameraModule()->GetProjectionMatrix()
            : float4x4::D3DOrthoProjLH(
                  -1, 1, (float)App->GetWindowModule()->GetWidth(), (float)App->GetWindowModule()->GetHeight()
              ); // near plane. far plane, screen width, screen height

    float width = 0;
    float3 startPos;
    if (transform2D != nullptr)
    {
        startPos = float3(transform2D->GetRenderingPosition(), 0) - parent->GetGlobalTransform().TranslatePart();
        width = transform2D->size.x;
    }
    glUniformMatrix4fv(0, 1, GL_TRUE, parent->GetGlobalTransform().ptr());
    glUniformMatrix4fv(1, 1, GL_TRUE, view.ptr());
    glUniformMatrix4fv(2, 1, GL_TRUE, proj.ptr());

    glUniform3fv(3, 1, fontColor.ptr()); // Font color

    glBindVertexArray(vao);
    TextManager::RenderText(*fontData, text, startPos, vbo, width);
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
            if (fontSize < 1) fontSize = 1;
            OnFontChange();
        }

        const char* preview = "Arial";
        if (ImGui::BeginCombo("combo 1", preview))
        {
            // TODO: Fonts could be a resource, so users can use the fonts they want and get loaded here

            // for (int n = 0; n < IM_ARRAYSIZE(items); n++)
            //{
            //     const bool is_selected = (item_selected_idx == n);
            //     if (ImGui::Selectable(items[n], is_selected)) item_selected_idx = n;
            //
            //     // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            //     if (is_selected) ImGui::SetItemDefaultFocus();
            // }
            ImGui::EndCombo();
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

void UILabelComponent::OnFontChange()
{
    fontData->Clean();
    fontData->Init("./EngineDefaults/Shader/Font/Arial.ttf", fontSize);
}