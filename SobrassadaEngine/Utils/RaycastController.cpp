#include "RaycastController.h"

#include "Application.h"
#include "FileSystem/Mesh.h"
#include "GameObject.h"
#include "SceneModule.h"
#include "Standalone/MeshComponent.h"

#include "Geometry/Triangle.h"
#include "Math/float4x4.h"
#include <algorithm>
#include <vector>

namespace RaycastController
{
    GameObject* GetRayIntersectionObject(const LineSegment& ray, const std::vector<GameObject*>& queriedGameObjects)
    {
        GameObject* selectedGameObject = nullptr;

        std::vector<GameObject*> aabbIntersectedObjects;

        // GETTING GAMEOBJECTS THAT INTERSECT WITH THE RAY
        float closeDistance = 0;
        float farDistance   = 0;
        for (const auto& gameObject : queriedGameObjects)
        {
            if (ray.Intersects(gameObject->GetGlobalAABB(), closeDistance, farDistance))
            {
                aabbIntersectedObjects.push_back(gameObject);
            }
        }

        float closestDistance = std::numeric_limits<float>::infinity();

        // FOREACH GAMEOBJECT INTERSECTING CHECKING AGAINST THE RAY
        for (const auto& gameObject : aabbIntersectedObjects)
        {
            LineSegment localRay(ray.a, ray.b);

            const MeshComponent* meshComponent = gameObject->GetMeshComponent();
            if (meshComponent == nullptr) continue; // TODO REMOVE WHEN EMPTY GAMEOBJECT DON'T CONTAIN AN AABB

            const ResourceMesh* resourceMesh = meshComponent->GetResourceMesh();

            float4x4 globalTransform = gameObject->GetGlobalTransform();
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
                        selectedGameObject = gameObject;
                    }
                }
            }
        }

        if (selectedGameObject)
        {
            const MeshComponent* meshComponent = selectedGameObject->GetMeshComponent();
            if (meshComponent && meshComponent->HasBones())
            {
                SceneModule* sceneModule     = App->GetSceneModule();

                UID rootGameObject           = sceneModule->GetGameObjectRootUID();
                GameObject* parentGameobject = sceneModule->GetGameObjectByUID(selectedGameObject->GetParent());

                while (parentGameobject->GetUID() != rootGameObject)
                {
                    selectedGameObject = parentGameobject;
                    parentGameobject   = sceneModule->GetGameObjectByUID(selectedGameObject->GetParent());
                }
            }
        }

        return selectedGameObject;
    }
}
