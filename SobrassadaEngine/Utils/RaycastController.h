#pragma once

#include "Geometry/LineSegment.h"
#include <vector>

namespace math
{
    class LineSegment;
}

class GameObject;

namespace RaycastController
{
    GameObject* GetRayIntersectionObject(const LineSegment& ray, const std::vector<GameObject*>& queriedGameObjects);

    template <typename... Tree> GameObject* GetRayIntersectionTrees(const LineSegment& ray, const Tree*... trees)
    {
        std::vector<GameObject*> queriedGameObjects;
        (trees->template QueryElements<LineSegment>(ray, queriedGameObjects), ...);

        return GetRayIntersectionObject(ray, queriedGameObjects);
    }
}