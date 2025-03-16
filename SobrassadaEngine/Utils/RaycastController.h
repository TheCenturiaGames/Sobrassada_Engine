#pragma once

#include "Geometry/LineSegment.h"
#include <vector>

namespace math
{
    class LineSegment;
}

class GameObject;

class RaycastController
{
  public:
    template <typename Tree> static GameObject* GetRayIntersectionSingleTree(const LineSegment& ray, const Tree* tree);
    template <typename... Tree> static GameObject* GetRayIntersectionTrees(const LineSegment& ray, const Tree*... trees);

  private:
    static GameObject* GetRayIntersectionObjects(const LineSegment& ray, const std::vector<GameObject*>& queriedGameObjects);
};

template <typename Tree>
inline GameObject* RaycastController::GetRayIntersectionSingleTree(const LineSegment& ray, const Tree* tree)
{
    GameObject* selectedGameObject = nullptr;
    if (tree == nullptr) return selectedGameObject;

    std::vector<GameObject*> queriedGameObjects;

    // GET ELEMENTS THAT INTERSECT WITH TREE
    tree->template QueryElements<LineSegment>(ray, queriedGameObjects);

    selectedGameObject = GetRayIntersectionObjects(ray, queriedGameObjects);

    return selectedGameObject;
}

template <typename... Tree>
inline GameObject* RaycastController::GetRayIntersectionTrees(const LineSegment& ray, const Tree*... trees)
{
    GameObject* selectedGameObject = nullptr;
    if (((trees == nullptr) || ...)) return selectedGameObject;

    std::vector<GameObject*> queriedGameObjects;
    (trees->template QueryElements<LineSegment>(ray, queriedGameObjects), ...);

    selectedGameObject = GetRayIntersectionObjects(ray, queriedGameObjects);

    return selectedGameObject;
}