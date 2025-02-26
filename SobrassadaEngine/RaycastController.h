#pragma once

#include "Geometry/LineSegment.h"
#include <vector>

class GameObject;
class Octree;
class Quadtree;

class RaycastController
{
  public:
    static void
    GetRayIntersections(const LineSegment& ray, const Octree* octree, std::vector<GameObject*>& outGameObjects);
    static void
    GetRayIntersections(const LineSegment& ray, const Quadtree* quadtree, std::vector<GameObject*>& outGameObjects);
};
