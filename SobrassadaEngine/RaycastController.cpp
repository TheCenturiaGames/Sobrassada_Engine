#include "RaycastController.h"

#include "Octree.h"
#include "Quadtree.h"

void RaycastController::GetRayIntersections(const LineSegment& ray, const Octree* octree, std::vector<GameObject*>& outGameObjects)
{
    if (octree == nullptr) return;
    octree->QueryElements<LineSegment>(ray, outGameObjects);
}

void RaycastController::GetRayIntersections(
   const LineSegment& ray, const Quadtree* quadtree, std::vector<GameObject*>& outGameObjects
)
{
    if (quadtree == nullptr) return;
    quadtree->QueryElements<LineSegment>(ray, outGameObjects);
}
