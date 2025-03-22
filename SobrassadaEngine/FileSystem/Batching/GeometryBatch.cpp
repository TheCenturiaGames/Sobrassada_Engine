#include "GeometryBatch.h"

#include <Application.h>
#include <Mesh.h>
#include <OpenGLModule.h>
#include <ResourceMesh.h>
#include <Standalone/MeshComponent.h>

#include <chrono>
#include <glew.h>

struct Command
{
    unsigned int count;         // Number of indices in the mesh
    unsigned int instanceCount; // Number of instances to render
    unsigned int firstIndex;    // Index offset in the EBO
    unsigned int baseVertex;    // Vertex offset in the VBO
    unsigned int baseInstance;  // Instance Index
};

GeometryBatch::GeometryBatch(const MeshComponent* mesh, const ResourceMesh* resource) : mode(resource->GetMode())
{
    glGenBuffers(1, &indirect);
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
}

GeometryBatch::~GeometryBatch()
{
    components.clear();
    uniqueMeshes.clear();
    glDeleteBuffers(1, &indirect);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
}

void GeometryBatch::LoadData()
{
    std::vector<Vertex> totalVertices;
    std::vector<unsigned int> totalIndices;

    for (const MeshComponent* component : components)
    {
        const ResourceMesh* resource = component->GetResourceMesh();
        const auto& it               = std::find(uniqueMeshes.begin(), uniqueMeshes.end(), resource);
        if (it != uniqueMeshes.end())
        {
            int idx                         = static_cast<int>(std::distance(uniqueMeshes.begin(), it));
            commands.at(idx).instanceCount += 1;
        }
        else
        {
            const std::vector<Vertex>& vertices      = resource->GetLocalVertices();
            const std::vector<unsigned int>& indices = resource->GetIndices();
            totalVertices.insert(totalVertices.end(), vertices.begin(), vertices.end());
            totalIndices.insert(totalIndices.end(), indices.begin(), indices.end());

            const unsigned int count = static_cast<unsigned int>(indices.size());

            Command newCommand;
            newCommand.count          = count;       // Number of indices in the mesh
            newCommand.instanceCount  = 1;           // Number of instances to render
            newCommand.firstIndex     = indexCount;  // Index offset in the EBO
            newCommand.baseVertex     = vertexCount; // Vertex offset in the VBO
            newCommand.baseInstance   = 0;           // Instance Index

            indexCount               += count;
            vertexCount              += static_cast<unsigned int>(vertices.size());
            commands.push_back(newCommand);
            uniqueMeshes.push_back(resource);
        }
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(Vertex), totalVertices.data(), GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0); // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    glEnableVertexAttribArray(1); // Tangent
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

    glEnableVertexAttribArray(2); // Normal
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glEnableVertexAttribArray(3); // Texture Coordinates
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    glEnableVertexAttribArray(4); // Joint
    glVertexAttribIPointer(4, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, joint));

    glEnableVertexAttribArray(5); // Weights
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), totalIndices.data(), GL_DYNAMIC_DRAW
    );

    glBindVertexArray(0);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, commands.size() * sizeof(Command), commands.data(), GL_DYNAMIC_DRAW);
}

void GeometryBatch::Render()
{
    const int meshTriangles = vertexCount / 3;
    const auto start        = std::chrono::high_resolution_clock::now();

    glBindVertexArray(vao);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect);
    glMultiDrawElementsIndirect(
        static_cast<GLenum>(mode), GL_UNSIGNED_INT, nullptr, static_cast<GLsizei>(commands.size()), 0
    );

    glBindVertexArray(0);

    const auto end                             = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<float> elapsed = end - start;

    App->GetOpenGLModule()->AddTrianglesPerSecond(meshTriangles / elapsed.count());
    App->GetOpenGLModule()->AddVerticesCount(vertexCount);
}

void GeometryBatch::ClearObjectsToRender()
{
    components.clear();
    uniqueMeshes.clear();
    commands.clear();
    vertexCount = 0;
    indexCount  = 0;
}