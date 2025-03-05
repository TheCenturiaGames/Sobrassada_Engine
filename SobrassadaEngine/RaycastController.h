#pragma once

#include "Application.h"
#include "ComponentUtils.h"
#include "GameObject.h"
#include "Geometry/LineSegment.h"
#include "Root/RootComponent.h"
#include "SceneModule.h"
#include "Standalone/MeshComponent.h"

#include "Geometry/Triangle.h"
#include "Math/float4x4.h"
#include <map>
#include <vector>

class GameObject;

class RaycastController
{
  public:
    template <typename Tree> static GameObject* GetRayIntersection(const LineSegment& ray, const Tree* tree);
};

template <typename Tree>
inline GameObject* RaycastController::GetRayIntersection(const LineSegment& ray, const Tree* tree)
{
    GameObject* selectedGameObject = nullptr;
    if (tree == nullptr) return selectedGameObject;

    std::vector<GameObject*> queriedGameObjects;

    // GET ELEMENTS THAT INTERSECT WITH OCTREE
    tree->template QueryElements<LineSegment>(ray, queriedGameObjects);

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

                float4x4 globalTransform = meshComponent->GetGlobalTransform();
                globalTransform.Inverse();
                localRay.Transform(globalTransform);

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
                            closestDistance    = distance;
                            selectedGameObject = pair.second;
                        }
                    }
                }
            }
        }
    }
    return selectedGameObject;
}
