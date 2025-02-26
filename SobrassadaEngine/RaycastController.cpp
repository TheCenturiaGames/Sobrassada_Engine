#include "RaycastController.h"

#include "Octree.h"
#include "Quadtree.h"

void RaycastController::GetRayIntersections(LineSegment& ray, Octree& octree, std::vector<GameObject*>& outGameObjects)
{
    octree.QueryElements<LineSegment>(ray, outGameObjects);
}

void RaycastController::GetRayIntersections(
    LineSegment& ray, Quadtree& quadtree, std::vector<GameObject*>& outGameObjects
)
{
    quadtree.QueryElements<LineSegment>(ray, outGameObjects);
}
