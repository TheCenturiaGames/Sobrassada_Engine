#include "BulletDebugDraw.h"

#include "Application.h"
#include "DebugDrawModule.h"
#include "Math/float3.h"

void BulletDebugDraw::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
    App->GetDebugDrawModule()->DrawLine(from, to, color);
}

void BulletDebugDraw::drawContactPoint(
    const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color
)
{
    App->GetDebugDrawModule()->DrawContactPoint(PointOnB, normalOnB, distance, lifeTime, color);
}

void BulletDebugDraw::draw3dText(const btVector3& location, const char* textString)
{
    App->GetDebugDrawModule()->Draw3DText(location, textString);
}

void BulletDebugDraw::reportErrorWarning(const char* warningString)
{
    GLOG(warningString)
}
