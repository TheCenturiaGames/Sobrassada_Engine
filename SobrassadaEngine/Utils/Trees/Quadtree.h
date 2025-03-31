#pragma once

#include "Geometry/AABB.h"
#include "Geometry/LineSegment.h"
#include "Math/float2.h"
#include "Math/float3.h"
#include "Math/float4.h"

#include <stack>
#include <vector>
#ifdef USE_OPTICK
#include "optick.h"
#endif

class GameObject;

class Quadtree
{
  private:
    struct QuadtreeElement
    {
        size_t id = -1;
        AABB boundingBox;
        GameObject* gameObject = nullptr;

        QuadtreeElement(const AABB& boundingBox, GameObject* gameObject, size_t id)
            : boundingBox(boundingBox), gameObject(gameObject), id(id) {};

        bool operator==(const QuadtreeElement& otherElement) const { return id == otherElement.id; }

        bool operator<(const QuadtreeElement& otherElement) const { return id < otherElement.id; }
    };

    class QuadtreeNode
    {
      public:
        QuadtreeNode() = default;
        QuadtreeNode(const AABB& currentArea, int capacity) : currentArea(currentArea), elementsCapacity(capacity) {};

        ~QuadtreeNode();

        void Subdivide();
        bool Intersects(const AABB& elementBoundingBox) const { return currentArea.Intersects(elementBoundingBox); };
        bool IsLeaf() const { return topLeft == nullptr; };

        AABB currentArea;
        int elementsCapacity = 0;

        std::vector<QuadtreeElement> elements;

        QuadtreeNode* topLeft     = nullptr;
        QuadtreeNode* topRight    = nullptr;
        QuadtreeNode* bottomLeft  = nullptr;
        QuadtreeNode* bottomRight = nullptr;
    };

  public:
    Quadtree(const float3& position, float size, int capacity);
    ~Quadtree();

    bool InsertElement(GameObject* newElement);
    const std::vector<LineSegment>& GetDrawLines();

    template <typename AreaType>
    void QueryElements(const AreaType& queryObject, std::vector<GameObject*>& foundElements) const;

  private:
    QuadtreeNode* rootNode;

    int totalLeaf     = 0;
    int totalElements = 0;

    std::vector<LineSegment> drawLines;
};

template <typename AreaType>
inline void Quadtree::QueryElements(const AreaType& queryObject, std::vector<GameObject*>& foundElements) const
{
#ifdef USE_OPTICK
    OPTICK_CATEGORY("Quadtree::QueryElements", Optick::Category::GameLogic)
#endif
    std::vector<bool> insertedElements = std::vector<bool>(totalElements, false);

    std::stack<const QuadtreeNode*> nodesToVisit;
    nodesToVisit.push(rootNode);

    while (!nodesToVisit.empty())
    {
        const QuadtreeNode* currentNode = nodesToVisit.top();
        nodesToVisit.pop();

        if (queryObject.Intersects(currentNode->currentArea))
        {
            if (currentNode->IsLeaf())
            {
                for (const auto& element : currentNode->elements)
                {
                    if (!insertedElements[element.id])
                    {
                        insertedElements[element.id] = true;
                        foundElements.push_back(element.gameObject);
                    }
                }
            }
            else
            {
                nodesToVisit.push(currentNode->topLeft);
                nodesToVisit.push(currentNode->topRight);
                nodesToVisit.push(currentNode->bottomLeft);
                nodesToVisit.push(currentNode->bottomRight);
            }
        }
    }
}