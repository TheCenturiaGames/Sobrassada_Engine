#include "Octree.h"

#include "GameObject.h"
#include "Standalone/MeshComponent.h"

#include "Geometry/LineSegment.h"
#include <set>
#include <stack>

Octree::OctreeNode::~OctreeNode()
{
    delete topLeftFront;
    delete topRightFront;
    delete bottomLeftFront;
    delete bottomRightFront;
    delete topLeftBack;
    delete topRightBack;
    delete bottomLeftBack;
    delete bottomRightBack;
}

void Octree::OctreeNode::Subdivide()
{
    float childSize          = currentArea.HalfSize().x;
    float3 center            = currentArea.CenterPoint();

    float3 pTop              = center + float3(0, childSize, 0);
    float3 pTopFront         = center + float3(0, childSize, childSize);
    float3 pTopRight         = center + float3(childSize, childSize, 0);
    float3 pTopFrontRight    = center + float3(childSize, childSize, childSize);
    float3 pBack             = center + float3(0, 0, -childSize);
    float3 pFront            = center + float3(0, 0, childSize);
    float3 pRight            = center + float3(childSize, 0, 0);
    float3 pLeft             = center + float3(-childSize, 0, 0);
    float3 pCenterBackLeft   = center + float3(-childSize, 0, -childSize);
    float3 pCenterFrontRight = center + float3(childSize, 0, childSize);
    float3 pBottom           = center + float3(0, -childSize, 0);
    float3 pBottomLeft       = center + float3(-childSize, -childSize, 0);
    float3 pBottomBack       = center + float3(0, -childSize, -childSize);
    float3 pBottomLeftBack   = center + float3(-childSize, -childSize, -childSize);

    topLeftFront             = new OctreeNode(AABB(pLeft, pTopFront), elementsCapacity);
    topRightFront            = new OctreeNode(AABB(center, pTopFrontRight), elementsCapacity);
    bottomLeftFront          = new OctreeNode(AABB(pBottomLeft, pFront), elementsCapacity);
    bottomRightFront         = new OctreeNode(AABB(pBottom, pCenterFrontRight), elementsCapacity);
    topLeftBack              = new OctreeNode(AABB(pCenterBackLeft, pTop), elementsCapacity);
    topRightBack             = new OctreeNode(AABB(pBack, pTopRight), elementsCapacity);
    bottomLeftBack           = new OctreeNode(AABB(pBottomLeftBack, center), elementsCapacity);
    bottomRightBack          = new OctreeNode(AABB(pBottomBack, pRight), elementsCapacity);

    for (auto& element : elements)
    {
        if (topLeftFront->Intersects(element.boundingBox)) topLeftFront->elements.push_back(element);
        if (topRightFront->Intersects(element.boundingBox)) topRightFront->elements.push_back(element);
        if (bottomLeftFront->Intersects(element.boundingBox)) bottomLeftFront->elements.push_back(element);
        if (bottomRightFront->Intersects(element.boundingBox)) bottomRightFront->elements.push_back(element);
        if (topLeftBack->Intersects(element.boundingBox)) topLeftBack->elements.push_back(element);
        if (topRightBack->Intersects(element.boundingBox)) topRightBack->elements.push_back(element);
        if (bottomLeftBack->Intersects(element.boundingBox)) bottomLeftBack->elements.push_back(element);
        if (bottomRightBack->Intersects(element.boundingBox)) bottomRightBack->elements.push_back(element);
    }

    elements.clear();
}

Octree::Octree(const float3& position, float size, int capacity)
{
    float halfSize       = size / 2.f;
    float3 minPosition   = float3(position.x - halfSize, position.y - halfSize, position.z - halfSize);
    float3 maxPosition   = float3(position.x + halfSize, position.y + halfSize, position.z + halfSize);

    AABB nodeBoundingBox = AABB(minPosition, maxPosition);

    rootNode             = new OctreeNode(nodeBoundingBox, capacity);
    totalLeaf            = 1;
}

Octree::~Octree()
{
    delete rootNode;
}

bool Octree::InsertElement(GameObject* gameObject)
{
    if (gameObject == nullptr) return false;

    bool inserted = false;
    std::stack<OctreeNode*> nodesToVisit;
    nodesToVisit.push(rootNode);

    const AABB objectBoundingBox = gameObject->GetGlobalAABB();
    OctreeElement octreeElement  = OctreeElement(objectBoundingBox, gameObject, totalElements);

    while (!nodesToVisit.empty())
    {
        OctreeNode* currentNode = nodesToVisit.top();
        nodesToVisit.pop();

        if (currentNode->Intersects(objectBoundingBox))
        {
            if (currentNode->IsLeaf())
            {
                if (currentNode->currentArea.HalfSize().x <= MINIMUM_TREE_LEAF_SIZE ||
                    currentNode->elements.size() < currentNode->elementsCapacity)
                {
                    currentNode->elements.push_back(octreeElement);
                    inserted = true;
                }
                else
                {
                    currentNode->Subdivide();
                    totalLeaf += 7;
                }
            }
            if (!currentNode->IsLeaf())
            {
                nodesToVisit.push(currentNode->topLeftFront);
                nodesToVisit.push(currentNode->topRightFront);
                nodesToVisit.push(currentNode->bottomLeftFront);
                nodesToVisit.push(currentNode->bottomRightFront);
                nodesToVisit.push(currentNode->topLeftBack);
                nodesToVisit.push(currentNode->topRightBack);
                nodesToVisit.push(currentNode->bottomLeftBack);
                nodesToVisit.push(currentNode->bottomRightBack);
            }
        }
    }
    if (inserted) ++totalElements;
    return inserted;

    return false;
}

const std::vector<LineSegment>& Octree::GetDrawLines()
{
    int totalLines = totalLeaf * 12;
    if (drawLines.size() == totalLines) return drawLines;

    drawLines = std::vector<LineSegment>(totalLines, LineSegment());

    std::stack<const OctreeNode*> nodesToVisit;
    nodesToVisit.push(rootNode);

    int currentDrawLine = 0;

    while (!nodesToVisit.empty())
    {
        const OctreeNode* currentNode = nodesToVisit.top();
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
            nodesToVisit.push(currentNode->topLeftFront);
            nodesToVisit.push(currentNode->topRightFront);
            nodesToVisit.push(currentNode->bottomLeftFront);
            nodesToVisit.push(currentNode->bottomRightFront);
            nodesToVisit.push(currentNode->topLeftBack);
            nodesToVisit.push(currentNode->topRightBack);
            nodesToVisit.push(currentNode->bottomLeftBack);
            nodesToVisit.push(currentNode->bottomRightBack);
        }
    }

    return drawLines;
}
