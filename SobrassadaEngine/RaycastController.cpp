#include "RaycastController.h"

#include "Octree.h"
#include "Quadtree.h"
#include "GameObject.h"

#include <map>

void RaycastController::GetRayIntersections(const LineSegment& ray, const Octree* octree, std::vector<GameObject*>& outGameObjects)
{
    if (octree == nullptr) return;
    std::vector<GameObject*> queriedGameObjects;

    octree->QueryElements<LineSegment>(ray, queriedGameObjects);
    
    std::map<float, GameObject*> sortedGameObjects;
    
    float closeDistance = 0;
    float farDistance = 0;
    for (const auto& gameObject : queriedGameObjects)
    {
        
        if (ray.Intersects(gameObject->GetAABB(), closeDistance, farDistance))
        {
            sortedGameObjects.insert({closeDistance, gameObject});
        }
    }

    int x = 0;
}

void RaycastController::GetRayIntersections(
   const LineSegment& ray, const Quadtree* quadtree, std::vector<GameObject*>& outGameObjects
)
{
    if (quadtree == nullptr) return;
    quadtree->QueryElements<LineSegment>(ray, outGameObjects);
}
