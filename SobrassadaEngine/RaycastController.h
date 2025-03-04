#pragma once

#include "Geometry/LineSegment.h"
#include <vector>

class GameObject;
class Octree;
class Quadtree;

class RaycastController
{
  public:
    static GameObject*
    GetRayIntersection(const LineSegment& ray, const Octree* octree);
};
