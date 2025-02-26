#include "RaycastController.h"

#include "Octree.h"
#include "Quadtree.h"
#include "GameObject.h"

#include "Math/float4x4.h"
#include "Math/Quat.h"
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

    for (const auto& pair : sortedGameObjects)
    {
        LineSegment localRay(ray.a, ray.b);
        
        Transform globalTransform = pair.second->GetGlobalTransform();
        Quat rotator      = Quat(globalTransform.rotation.x, globalTransform.rotation.y, globalTransform.rotation.z, 1);
        float4x4 modelMat = float4x4::FromTRS(globalTransform.position, rotator, globalTransform.scale);
        localRay.Transform(modelMat);
        
    }
}

void RaycastController::GetRayIntersections(
   const LineSegment& ray, const Quadtree* quadtree, std::vector<GameObject*>& outGameObjects
)
{
    if (quadtree == nullptr) return;
    quadtree->QueryElements<LineSegment>(ray, outGameObjects);
}
