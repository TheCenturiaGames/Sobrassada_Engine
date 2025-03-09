#include "Octree.h"

#include "FrustumPlanes.h"
#include "GameObject.h"

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
    const AABB objectBoundingBox = gameObject->GetAABB();
    OctreeElement octreeElement  = OctreeElement(objectBoundingBox, gameObject, totalElements);
    
    struct InsertionData
    {
        Octree::OctreeNode* node;
        Octree::OctreeElement element;
    };

    std::stack<InsertionData> dataToVisit;
    dataToVisit.push({rootNode, octreeElement});


     while (!dataToVisit.empty())
    {
        InsertionData current = dataToVisit.top();
        dataToVisit.pop();

        // Si l'AABB de l'element interseca amb l'espai del node
        if (current.node->Intersects(current.element.boundingBox))
        {
            if (current.node->IsLeaf())
            {
                // Si el node ha arribat a la mida mínima o encara té capacitat, s'insereix
                if (current.node->currentArea.HalfSize().x <= MinimumLeafSize ||
                    current.node->elements.size() < current.node->elementsCapacity)
                {
                    current.node->elements.push_back(current.element);
                    inserted = true;
                }
                else
                {
                    // Si el node és fulla i està ple: subdivideix
                    current.node->Subdivide();
                    totalLeaf += 7; // actualitzem el nombre de fulles

                    // Reinserim l'element en tots els fills
                    dataToVisit.push({current.node->topLeftFront, current.element});
                    dataToVisit.push({current.node->topRightFront, current.element});
                    dataToVisit.push({current.node->bottomLeftFront, current.element});
                    dataToVisit.push({current.node->bottomRightFront, current.element});
                    dataToVisit.push({current.node->topLeftBack, current.element});
                    dataToVisit.push({current.node->topRightBack, current.element});
                    dataToVisit.push({current.node->bottomLeftBack, current.element});
                    dataToVisit.push({current.node->bottomRightBack, current.element});
                }
            }
            else
            {
                // Si el node ja té fills, ens assegurem de processar tots els fills
                dataToVisit.push({current.node->topLeftFront, current.element});
                dataToVisit.push({current.node->topRightFront, current.element});
                dataToVisit.push({current.node->bottomLeftFront, current.element});
                dataToVisit.push({current.node->bottomRightFront, current.element});
                dataToVisit.push({current.node->topLeftBack, current.element});
                dataToVisit.push({current.node->topRightBack, current.element});
                dataToVisit.push({current.node->bottomLeftBack, current.element});
                dataToVisit.push({current.node->bottomRightBack, current.element});
            }
        }
    }
    if (inserted) ++totalElements;
    return inserted;
}

void Octree::QueryElements(const AABB& area, std::vector<GameObject*>& foundElements) const
{
    std::vector<bool> insertedElements = std::vector<bool>(totalElements, false);

    std::stack<OctreeNode*> nodesToVisit;
    nodesToVisit.push(rootNode);

    while (!nodesToVisit.empty())
    {
        const OctreeNode* currentNode = nodesToVisit.top();
        nodesToVisit.pop();

        if (currentNode->Intersects(area))
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
}

void Octree::QueryElements(const FrustumPlanes& cameraPlanes, std::vector<GameObject*>& foundElements) const
{
    std::vector<bool> insertedElements = std::vector<bool>(totalElements, false);

    std::stack<OctreeNode*> nodesToVisit;
    nodesToVisit.push(rootNode);

    while (!nodesToVisit.empty())
    {
        const OctreeNode* currentNode = nodesToVisit.top();
        nodesToVisit.pop();

        if (cameraPlanes.Intersects(currentNode->currentArea))
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
}

void Octree::GetDrawLines(std::vector<LineSegment>& drawLines, std::vector<LineSegment>& elementLines) const
{
    std::set<OctreeElement> includedElement;
    drawLines    = std::vector<LineSegment>(totalLeaf * 12, LineSegment());
    elementLines = std::vector<LineSegment>(totalElements * 12, LineSegment());

    std::stack<const OctreeNode*> nodesToVisit;
    nodesToVisit.push(rootNode);

    int currentDrawLine    = 0;
    int currentElementLine = 0;

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

            for (const auto& element : currentNode->elements)
            {
                if (includedElement.find(element) == includedElement.end())
                {
                    includedElement.insert(element);

                    for (int i = 0; i < 12; ++i)
                    {
                        elementLines[currentElementLine++] = element.boundingBox.Edge(i);
                    }
                }
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
}
