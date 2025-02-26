#pragma once

#include "Geometry/LineSegment.h"
#include <vector>

class GameObject;
class Octree;
class Quadtree;

class RaycastController
{
  public:
    static void GetRayIntersections(LineSegment& ray, Octree& octree, std::vector<GameObject*>& outGameObjects);
    static void GetRayIntersections(LineSegment& ray, Quadtree& quadtree, std::vector<GameObject*>& outGameObjects);
};

