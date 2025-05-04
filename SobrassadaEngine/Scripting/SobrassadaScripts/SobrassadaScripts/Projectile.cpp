#include "pch.h"

#include "Projectile.h"

#include "GameObject.h"
#include "Math/Quat.h"

Projectile::Projectile(GameObject* parent) : Script(parent)
{
    fields.push_back({"Speed", InspectorField::FieldType::Float, &speed, 0, 100});
    fields.push_back({"Range", InspectorField::FieldType::Float, &range, 0, 100});
}

bool Projectile::Init()
{
    collider = parent->GetComponent<CubeColliderComponent*>();
    if (!collider)
    {
        GLOG("[WARNING: Projectile Init()] Couldn't find the collider component");
        return false;
    }
}

void Projectile::Update(float deltaTime)
{
    Move(deltaTime);
}

void Projectile::Shoot(const float3& origin, const float3& direction)
{
    GLOG("Shoot to dir: %f %f %f", direction.x, direction.y, direction.z);
    parent->SetEnabled(true);
    parent->SetLocalPosition(origin);
    startPos           = origin;
    this->direction    = direction;

    // Rotate spear object
    const float3 scale = parent->GetLocalTransform().ExtractScale();
    Quat rotation      = Quat::LookAt(float3::unitZ, direction, float3::unitY, float3::unitY);
    float4x4 transform = float4x4::FromTRS(origin, rotation, scale);
    parent->SetLocalTransform(transform);
}

void Projectile::Move(float deltaTime)
{
    float3 currentPos  = parent->GetPosition();
    currentPos        += direction * speed * deltaTime;
    parent->SetLocalPosition(currentPos);

    if (currentPos.Distance(startPos) > range) parent->SetEnabled(false);
}