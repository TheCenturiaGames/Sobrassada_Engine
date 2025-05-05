#include "CanvasScalerComponent.h"
#include "Application.h"
#include "CanvasComponent.h"
#include "GameObject.h"
#include "Transform2DComponent.h"
#include "WindowModule.h"

CanvasScalerComponent::CanvasScalerComponent(UID uid, GameObject* parent)
    : Component(uid, parent, "Canvas Scaler", STATIC_TYPE)
{
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
