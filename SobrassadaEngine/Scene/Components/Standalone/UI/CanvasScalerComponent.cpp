#include "CanvasScalerComponent.h"
#include "Application.h"
#include "CanvasComponent.h"
#include "GameObject.h"
#include "Transform2DComponent.h"
#include "WindowModule.h"
#include "imgui.h"

CanvasScalerComponent::CanvasScalerComponent(UID uid, GameObject* parent)
    : Component(uid, parent, "Canvas Scaler", STATIC_TYPE)
{
}

CanvasScalerComponent::CanvasScalerComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent)
{
    if (initialState.HasMember("ReferenceResolution") && initialState["ReferenceResolution"].IsArray())
    {
        const rapidjson::Value& refRes = initialState["ReferenceResolution"];
        referenceResolution.x          = refRes[0].GetFloat();
        referenceResolution.y          = refRes[1].GetFloat();
    }
}


void CanvasScalerComponent::Init()
{
    canvas      = parent->GetComponent<CanvasComponent*>();
    transform2D = parent->GetComponent<Transform2DComponent*>();

    if (!canvas || !transform2D)
    {
        GLOG("[ERROR] CanvasScalerComponent requires a CanvasComponent and Transform2DComponent.");
        return;
    }
}

void CanvasScalerComponent::Update(float deltaTime)
{
    if (!canvas || !transform2D) return;

    float currentWidth  = (float)App->GetWindowModule()->GetWidth();
    float currentHeight = (float)App->GetWindowModule()->GetHeight();

    float scaleX        = currentWidth / referenceResolution.x;
    float scaleY        = currentHeight / referenceResolution.y;
    float scale         = std::min(scaleX, scaleY);

    transform2D->size   = referenceResolution * scale;
}


void CanvasScalerComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);

    rapidjson::Value valRefRes(rapidjson::kArrayType);
    valRefRes.PushBack(referenceResolution.x, allocator);
    valRefRes.PushBack(referenceResolution.y, allocator);
    targetState.AddMember("ReferenceResolution", valRefRes, allocator);
}

void CanvasScalerComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();

    ImGui::SeparatorText("Canvas Scaler");

    if (ImGui::DragFloat2("Reference Resolution", &referenceResolution[0], 1.0f, 1.0f, 10000.0f))
    {
        if (canvas && transform2D)
        {
            float currentWidth  = (float)App->GetWindowModule()->GetWidth();
            float currentHeight = (float)App->GetWindowModule()->GetHeight();

            float scaleX        = currentWidth / referenceResolution.x;
            float scaleY        = currentHeight / referenceResolution.y;
            float scale         = std::min(scaleX, scaleY);

            transform2D->size   = referenceResolution * scale;
        }
    }
}
