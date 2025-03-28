#include "BulletMotionState.h"

#include "Application.h"
#include "Component.h"
#include "GameObject.h"
#include "SceneModule.h"

#include "Math/float3.h"
#include "Math/float4x4.h"
#include "Math/Quat.h"

BulletMotionState::BulletMotionState(
    Component* newCollider, const math::float3& newCenterOffset, const math::float3& newCenterRotation,
    bool newFreezeRotation
)
    : collider(newCollider), freezeRotation(newFreezeRotation)
{
    btQuaternion rotationQuat =
        btQuaternion(btScalar(newCenterRotation.y), btScalar(newCenterRotation.x), btScalar(newCenterRotation.z));

    centerOffset = btTransform(
        rotationQuat, btVector3(btScalar(newCenterOffset.x), btScalar(newCenterOffset.y), btScalar(newCenterOffset.z))
    );
}

// Syncronize from render world to physics world
void BulletMotionState::getWorldTransform(btTransform& outPhysicsWorldTransform) const
{
    GameObject* parent = collider->GetParent();

    btTransform gameObjectWorldTransform;
    gameObjectWorldTransform.setFromOpenGLMatrix(parent->GetGlobalTransform().ptr());

    outPhysicsWorldTransform = gameObjectWorldTransform * centerOffset.inverse();
}

// Syncronize from physics world to render world
void BulletMotionState::setWorldTransform(const btTransform& physicsWorldTransform)
{
    if (!App->GetSceneModule()->GetInPlayMode()) return;

    // Physics world transform
    btTransform finalPhysicsTransform;
    finalPhysicsTransform.setIdentity();

    if (!freezeRotation) finalPhysicsTransform.setBasis(physicsWorldTransform.getBasis());
    finalPhysicsTransform.setOrigin(physicsWorldTransform.getOrigin());

    // Render world physics collider transform
    btTransform finalOffsetTransform;
    finalOffsetTransform.setIdentity();

    if (!freezeRotation) finalOffsetTransform.setBasis(centerOffset.getBasis());
    finalOffsetTransform.setOrigin(centerOffset.getOrigin());

    btTransform gameObjectTransform = finalPhysicsTransform * finalOffsetTransform;

    float3x3 rotationMat;
    gameObjectTransform.getOpenGLMatrix(rotationMat.ptr());

    float3 newPosition = float3(
        gameObjectTransform.getOrigin().x(), gameObjectTransform.getOrigin().y(), gameObjectTransform.getOrigin().z()
    );

    float4x4 newLocalMatrix = float4x4::FromTRS(newPosition, rotationMat, collider->GetParent()->GetScale());

    collider->GetParent()->SetLocalTransform(newLocalMatrix);
    collider->GetParent()->OnTransformUpdated();
}
