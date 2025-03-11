#pragma once

#include "Math/float2.h"
#include "Math/float3.h"
#include "Math/float4.h"
#include <vector>

struct Vertex
{
    float3 position;
    float4 tangent;
    unsigned int joint[4];
    float4 weights;
    float3 normal;
    float2 texCoord;

    Vertex& operator=(const Vertex& other)
    {
        if (this != &other)
        {
            position = other.position;
            tangent  = other.tangent;
            normal   = other.normal;
            texCoord = other.texCoord;
        }
        return *this;
    }
};

class Mesh
{
  public:
    Mesh() = default;

    const std::vector<Vertex>& GetVertices() const { return vertices; }
    const std::vector<unsigned int>& GetIndices() const { return indices; }
    unsigned int GetMode() const { return mode; }
    // unsigned int GetMaterialIndex() const { return materialIndex; }

    // 4 bytes -> cheap, move not worth it
    // void SetMaterialIndex(unsigned int newMaterialIndex) { materialIndex = newMaterialIndex; }
    void SetMode(unsigned int newMode) { mode = newMode; }
    // expensive -> move operators
    void SetVertices(std::vector<Vertex>&& newVertices) { vertices = std::move(newVertices); }
    void SetIndices(std::vector<unsigned int>&& newIndices) { indices = std::move(newIndices); }

  private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    // unsigned int materialIndex = 0;
    unsigned int mode = 4; // Default: GL_TRIANGLES
};