#include "RaycastController.h"

#include "GameObject.h"
#include "Standalone/MeshComponent.h"

#include "Geometry/Triangle.h"
#include "Math/float4x4.h"
#include <map>

GameObject*
RaycastController::GetRayIntersectionObjects(const LineSegment& ray, const std::vector<GameObject*>& queriedGameObjects)
{
    GameObject* selectedGameObject = nullptr;

    std::map<float, GameObject*> sortedGameObjects;

    // GETTING GAMEOBJECTS THAT INTERSECT WITH THE RAY AND SORT THEM BY DISTANCE
    float closeDistance = 0;
    float farDistance   = 0;
    for (const auto& gameObject : queriedGameObjects)
    {
        if (ray.Intersects(gameObject->GetGlobalAABB(), closeDistance, farDistance))
        {
            sortedGameObjects.insert({closeDistance, gameObject});
        }
    }

    float closestDistance = std::numeric_limits<float>::infinity();

    // FOREACH GAMEOBJECT INTERSECTING CHECKING AGAINST THE RAY
    for (const auto& pair : sortedGameObjects)
    {
        LineSegment localRay(ray.a, ray.b);

        const MeshComponent* meshComponent = pair.second->GetMeshComponent();
        if (meshComponent == nullptr) continue; // TODO Happens occasionally, figure out why

        const ResourceMesh* resourceMesh = meshComponent->GetResourceMesh();

        if (resourceMesh == nullptr) continue;

        float4x4 globalTransform = pair.second->GetGlobalTransform();
        globalTransform.Inverse();
        localRay.Transform(globalTransform);

        const std::vector<Vertex>& vertices      = resourceMesh->GetLocalVertices();
        const std::vector<unsigned int>& indices = resourceMesh->GetIndices();

        for (int vertexIndex = 2; vertexIndex < indices.size(); vertexIndex += 3)
        {
            float3 firstVertex  = vertices[indices[vertexIndex - 2]].position;
            float3 secondVertex = vertices[indices[vertexIndex - 1]].position;
            float3 thirdVertex  = vertices[indices[vertexIndex]].position;

            Triangle currentTriangle(firstVertex, secondVertex, thirdVertex);

            float distance = std::numeric_limits<float>::infinity();
            float3 hitPoint;

            if (localRay.Intersects(currentTriangle, &distance, &hitPoint))
            {
                if (distance < closestDistance)
                {
                    closestDistance    = distance;
                    selectedGameObject = pair.second;
                }
            }
        }
    }
    return selectedGameObject;
}
