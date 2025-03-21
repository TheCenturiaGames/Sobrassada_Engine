#include "Quadtree.h"

#include "GameObject.h"
#include "Standalone/MeshComponent.h"

#include <set>

Quadtree::Quadtree(const float3& position, float size, int capacity)
{
    float halfSize       = size / 2.f;
    float3 minPosition   = float3(position.x - halfSize, 0, position.z - halfSize);
    float3 maxPosition   = float3(position.x + halfSize, 0, position.z + halfSize);
    AABB nodeBoundingBox = AABB(minPosition, maxPosition);

    rootNode             = new QuadtreeNode(nodeBoundingBox, capacity);
    totalLeaf            = 1;
}

Quadtree::~Quadtree()
{
    delete rootNode;
}

bool Quadtree::InsertElement(GameObject* gameObject)
{
    if (gameObject == nullptr) return false;

    bool inserted = false;
    std::stack<QuadtreeNode*> nodesToVisit;
    nodesToVisit.push(rootNode);

    AABB elementBoundingBox   = AABB(gameObject->GetGlobalAABB());
    elementBoundingBox.minPoint.y   = -1;
    elementBoundingBox.maxPoint.y   = 1;
    QuadtreeElement quadtreeElement = QuadtreeElement(elementBoundingBox, gameObject, totalElements);

    while (!nodesToVisit.empty())
    {
        QuadtreeNode* currentNode = nodesToVisit.top();
        nodesToVisit.pop();

        if (currentNode->Intersects(elementBoundingBox))
        {
            if (currentNode->IsLeaf())
            {
                if (currentNode->currentArea.Size().x <= MINIMUM_TREE_LEAF_SIZE ||
                    currentNode->elements.size() < currentNode->elementsCapacity)
                {
                    currentNode->elements.push_back(quadtreeElement);
                    inserted = true;
                }
                else
                {
                    currentNode->Subdivide();
                    totalLeaf += 3;
                }
            }
            if (!currentNode->IsLeaf())
            {
                nodesToVisit.push(currentNode->topLeft);
                nodesToVisit.push(currentNode->topRight);
                nodesToVisit.push(currentNode->bottomLeft);
                nodesToVisit.push(currentNode->bottomRight);
            }
        }
    }
    if (inserted) ++totalElements;
    return inserted;
}

const std::vector<LineSegment>& Quadtree::GetDrawLines()
{
    int totalLines = totalLeaf * 12;
    if (drawLines.size() == totalLines) return drawLines;

    drawLines = std::vector<LineSegment>(totalLines, LineSegment());

    std::stack<const QuadtreeNode*> nodesToVisit;
    nodesToVisit.push(rootNode);

    int currentDrawLine    = 0;
    int currentElementLine = 0;

    while (!nodesToVisit.empty())
    {
        const QuadtreeNode* currentNode = nodesToVisit.top();
        nodesToVisit.pop();

        if (currentNode->IsLeaf())
        {
            for (int i = 0; i < 12; ++i)
            {
                drawLines[currentDrawLine++] = currentNode->currentArea.Edge(i);
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

    return drawLines;
}

Quadtree::QuadtreeNode::~QuadtreeNode()
{

    delete topLeft;
    delete topRight;
    delete bottomLeft;
    delete bottomRight;
}

void Quadtree::QuadtreeNode::Subdivide()
{
    float childSize        = currentArea.Size().x / 2.f;
    float3 center          = currentArea.CenterPoint();

    float3 topPoint        = center + float3(0, 0, childSize);
    float3 topRightPoint   = center + float3(childSize, 0, childSize);
    float3 rightPoint      = center + float3(childSize, 0, 0);
    float3 bottomPoint     = center + float3(0, 0, -childSize);
    float3 bottomLeftPoint = center + float3(-childSize, 0, -childSize);
    float3 leftPoint       = center + float3(-childSize, 0, 0);

    topLeft                = new QuadtreeNode(AABB(leftPoint, topPoint), elementsCapacity);
    topRight               = new QuadtreeNode(AABB(center, topRightPoint), elementsCapacity);
    bottomLeft             = new QuadtreeNode(AABB(bottomLeftPoint, center), elementsCapacity);
    bottomRight            = new QuadtreeNode(AABB(bottomPoint, rightPoint), elementsCapacity);

    for (auto& element : elements)
    {
        if (topLeft->Intersects(element.boundingBox)) topLeft->elements.push_back(element);
        if (topRight->Intersects(element.boundingBox)) topRight->elements.push_back(element);
        if (bottomLeft->Intersects(element.boundingBox)) bottomLeft->elements.push_back(element);
        if (bottomRight->Intersects(element.boundingBox)) bottomRight->elements.push_back(element);
    }

    elements.clear();
}