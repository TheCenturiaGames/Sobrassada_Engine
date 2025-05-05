#include "pch.h"

#include "Projectile.h"

#include "Character.h"
#include "GameObject.h"
#include "ScriptComponent.h"
#include "Standalone/Physics/CubeColliderComponent.h"

#include "Math/Quat.h"

Projectile::Projectile(GameObject* parent) : Script(parent)
{
    fields.push_back({"Speed", InspectorField::FieldType::Float, &speed, 0.0f, 100.0f});
    fields.push_back({"Range", InspectorField::FieldType::Float, &range, 0.0f, 100.0f});
    fields.push_back({"Damage", InspectorField::FieldType::Int, &damage, 0, 10});
}

bool Projectile::Init()
{
    collider = parent->GetComponent<CubeColliderComponent*>();
    if (!collider)
    {
        GLOG("[WARNING: Projectile Init()] Couldn't find the collider component");
        return false;
    }
    return true;
}

void Projectile::Update(float deltaTime)
{
    Move(deltaTime);
}

void Projectile::Shoot(const float3& origin, const float3& direction)
{
    GLOG("Shoot to dir: %f %f %f", direction.x, direction.y, direction.z);
    startPos        = origin;
    this->direction = direction;
    frames          = 0;
    parent->SetEnabled(true);

    // Rotate spear object
    const float3 scale       = parent->GetLocalTransform().ExtractScale();
    const Quat rotation      = Quat::LookAt(float3::unitZ, direction, float3::unitY, float3::unitY);
    const float4x4 transform = float4x4::FromTRS(origin, rotation, scale);
    parent->SetLocalTransform(transform);
}

void Projectile::OnCollision(GameObject* otherObject, const float3& collisionNormal)
{
    GLOG("COLLISION IN PROJECTILE with: %s", otherObject->GetName().c_str());

    collider->SetEnabled(false);
    parent->SetEnabled(false);
}

void Projectile::Move(float deltaTime)
{
    // Let 20 frames pass before enabling the collider, so it doesn't collide with the previous collided element.
    // TODO: Try to change this
    frames += 1;
    if (frames > 20 && !collider->GetEnabled()) collider->SetEnabled(true);

    float3 currentPos  = parent->GetPosition();
    currentPos        += direction * speed * deltaTime;
    parent->SetLocalPosition(currentPos);

    if (currentPos.Distance(startPos) > range) parent->SetEnabled(false);
}