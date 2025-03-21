#pragma once

#include <Math/float2.h>
#include <Math/float3.h>
#include <Math/float4.h>
#include <vector>

// ORDER MATTERS
struct Vertex
{
    float3 position;
    float4 tangent;
    float3 normal;
    float2 texCoord;
    unsigned int joint[4];
    float4 weights;

    Vertex& operator=(const Vertex& other)
    {
        if (this != &other)
        {
            position = other.position;
            tangent  = other.tangent;
            normal   = other.normal;
            texCoord = other.texCoord;
            joint[0] = other.joint[0];
            joint[1] = other.joint[1];
            joint[2] = other.joint[2];
            joint[3] = other.joint[3];
            weights  = other.weights;
        }
        return *this;
    }
};

class Mesh
{
  public:
    Mesh()  = default;
    ~Mesh() = default;

    const std::vector<Vertex>& GetVertices() const { return vertices; }
    const std::vector<unsigned int>& GetIndices() const { return indices; }
    unsigned int GetMode() const { return mode; }

    void SetMode(unsigned int newMode) { mode = newMode; }
    // expensive -> move operators
    void SetVertices(std::vector<Vertex>&& newVertices) { vertices = std::move(newVertices); }
    void SetIndices(std::vector<unsigned int>&& newIndices) { indices = std::move(newIndices); }

  private:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    unsigned int mode = 4; // Default: GL_TRIANGLES
};