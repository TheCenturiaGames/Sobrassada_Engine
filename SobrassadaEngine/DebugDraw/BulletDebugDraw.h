#pragma once

#include "btIDebugDraw.h"

class BulletDebugDraw : public btIDebugDraw
{
  public:
    BulletDebugDraw()  = default;
    ~BulletDebugDraw() = default;

    void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
    void drawContactPoint(
        const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color
    ) override;
    void draw3dText(const btVector3& location, const char* textString) override;
    void reportErrorWarning(const char* warningString) override;

    int getDebugMode() const override { return debugMode; };
    void setDebugMode(int newDebugMode) override { debugMode = newDebugMode; };

  private:
    int debugMode = DebugDrawModes::DBG_NoDebug;
};
