#include "RaycastController.h"

#include "Application.h"
#include "ComponentUtils.h"
#include "GameObject.h"
#include "Octree.h"
#include "Quadtree.h"
#include "Root/RootComponent.h"
#include "SceneModule.h"
#include "Standalone/MeshComponent.h"

#include "Geometry/Triangle.h"
#include "Math/Quat.h"
#include "Math/float4x4.h"
#include <map>

void RaycastController::GetRayIntersections(
    const LineSegment& ray, const Octree* octree, std::vector<GameObject*>& outGameObjects
)
{
    if (octree == nullptr) return;
    std::vector<GameObject*> queriedGameObjects;
    GameObject* selectedGameObject = nullptr;

    // GET ELEMENTS THAT INTERSECT WITH OCTREE
    octree->QueryElements<LineSegment>(ray, queriedGameObjects);

    std::map<float, GameObject*> sortedGameObjects;

    // GETTING GAMEOBJECTS THAT INTERSECT WITH THE RAY AND SORT THEM BY DISTANCE
    float closeDistance = 0;
    float farDistance   = 0;
    for (const auto& gameObject : queriedGameObjects)
    {

        if (ray.Intersects(gameObject->GetAABB(), closeDistance, farDistance))
        {
            sortedGameObjects.insert({closeDistance, gameObject});
        }
    }
    float closestDistance = std::numeric_limits<float>::infinity();

    // FOREACH GAMEOBJECT INTERSECTING CHECKING AGAINST THE RAY
    for (const auto& pair : sortedGameObjects)
    {
        Component* currentComponent      = nullptr;
        std::vector<UID> childComponents = pair.second->GetRootComponent()->GetChildren();

        for (UID componentUID : childComponents)
        {
            currentComponent = App->GetSceneModule()->GetComponentByUID(componentUID);
            if (currentComponent != nullptr && currentComponent->GetType() == ComponentType::COMPONENT_MESH)
            {
                LineSegment localRay(ray.a, ray.b);

                MeshComponent* meshComponent     = reinterpret_cast<MeshComponent*>(currentComponent);
                const ResourceMesh* resourceMesh = meshComponent->GetResourceMesh();

                if (resourceMesh == nullptr) continue;

                Transform globalTransform = meshComponent->GetGlobalTransform();
                Quat rotator =
                    Quat(globalTransform.rotation.x, globalTransform.rotation.y, globalTransform.rotation.z, 1);
                float4x4 modelMat = float4x4::FromTRS(globalTransform.position, rotator, globalTransform.scale);
                modelMat.Inverse();
                localRay.Transform(modelMat);

                std::vector<Vertex> vertices      = resourceMesh->GetLocalVertices();
                std::vector<unsigned int> indices = resourceMesh->GetIndices();

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
                            closestDistance = distance;
                            selectedGameObject = pair.second;
                        }
                    }
                }
            }
        }
    }
}

void RaycastController::GetRayIntersections(
    const LineSegment& ray, const Quadtree* quadtree, std::vector<GameObject*>& outGameObjects
)
{
    if (quadtree == nullptr) return;
    quadtree->QueryElements<LineSegment>(ray, outGameObjects);
}
