#pragma once

#include "Math/float2.h"
#include "Resource.h"
#include <vector>

struct ControlPoint
{
    float2 position;
    float tangentAngle; 
};

class ResourceCurve : public Resource
{
  public:
    ResourceCurve(UID uid, const std::string& name);
    ~ResourceCurve() override = default;

    std::vector<ControlPoint> controlPoints;

    void Save(const std::string& path) const;
    bool Load(const std::string& path);
};