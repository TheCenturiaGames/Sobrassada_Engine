#include "GeometryBatch.h"

#include <Application.h>
#include <Mesh.h>
#include <OpenGLModule.h>
#include <ResourceMaterial.h>
#include <ResourceMesh.h>
#include <ShaderModule.h>
#include <Standalone/MeshComponent.h>

#include <chrono>
#include <glew.h>
#include <numeric>

struct Command
{
    unsigned int count;         // Number of indices in the mesh
    unsigned int instanceCount; // Number of instances to render
    unsigned int firstIndex;    // Index offset in the EBO
    unsigned int baseVertex;    // Vertex offset in the VBO
    unsigned int baseInstance;  // Instance Index
};

GeometryBatch::GeometryBatch(const MeshComponent* component) : mode(component->GetResourceMesh()->GetMode())
{
    isMetallic = component->GetResourceMaterial()->GetIsMetallicRoughness();
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &indirect);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenBuffers(1, &models);
    glGenBuffers(1, &materials);
}

GeometryBatch::~GeometryBatch()
{
    components.clear();
    uniqueMeshes.clear();
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &indirect);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &models);
    glDeleteBuffers(1, &materials);
}

void GeometryBatch::LoadData()
{
    std::vector<Vertex> totalVertices;
    std::vector<unsigned int> totalIndices;

    for (const MeshComponent* component : components)
    {
        const ResourceMesh* resource = component->GetResourceMesh();
        const auto& it               = uniqueMeshes.find(resource);
        if (it == uniqueMeshes.end())
        {
            const std::vector<Vertex>& vertices      = resource->GetLocalVertices();
            const std::vector<unsigned int>& indices = resource->GetIndices();
            totalVertices.insert(totalVertices.end(), vertices.begin(), vertices.end());
            totalIndices.insert(totalIndices.end(), indices.begin(), indices.end());
            uniqueMeshes.insert(resource);
            MeshCount newMeshCount;
            newMeshCount.vertexCount = resource->GetVertexCount();
            newMeshCount.indexCount  = resource->GetIndexCount();
            uniqueMeshesCount.push_back(newMeshCount);
        }
    }

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, totalVertices.size() * sizeof(Vertex), totalVertices.data(), GL_DYNAMIC_DRAW);

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
        GL_ELEMENT_ARRAY_BUFFER, totalIndices.size() * sizeof(unsigned int), totalIndices.data(), GL_DYNAMIC_DRAW
    );

    glBindVertexArray(0);
}

void GeometryBatch::Render(
    unsigned int program, unsigned int cameraUBO, float3 cameraPos, const std::vector<MeshComponent*>& meshesToRender
)
{
    const auto start = std::chrono::high_resolution_clock::now();

    std::vector<float4x4> totalModels;
    std::vector<MaterialGPU> totalMaterials;
    std::vector<Command> commands;

    GenerateCommandsAndSSBO(meshesToRender, commands, totalModels, totalMaterials);

    const int meshTriangles = totalVertexCount / 3;

    glUseProgram(program);

    glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
    unsigned int blockIdx = glGetUniformBlockIndex(program, "CameraMatrices");
    glUniformBlockBinding(program, blockIdx, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glUniform3fv(glGetUniformLocation(program, "cameraPos"), 1, &cameraPos[0]);

    glUniform1i(4, 0); // hasBones

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, commands.size() * sizeof(Command), commands.data(), GL_DYNAMIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, models);
    glBufferData(GL_SHADER_STORAGE_BUFFER, totalModels.size() * sizeof(float4x4), totalModels.data(), GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, models);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, materials);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER, totalMaterials.size() * sizeof(MaterialGPU), totalMaterials.data(), GL_DYNAMIC_DRAW
    );
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, materials);

    glBindVertexArray(vao);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect);
    glMultiDrawElementsIndirect(
        static_cast<GLenum>(mode), GL_UNSIGNED_INT, (GLvoid*)0, static_cast<GLsizei>(commands.size()), 0
    );

    glBindVertexArray(0);

    const auto end                             = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<float> elapsed = end - start;

    App->GetOpenGLModule()->AddTrianglesPerSecond(meshTriangles / elapsed.count());
    App->GetOpenGLModule()->AddVerticesCount(totalVertexCount);
}

void GeometryBatch::GenerateCommandsAndSSBO(
    const std::vector<MeshComponent*>& meshes, std::vector<Command>& commands, std::vector<float4x4>& totalModels,
    std::vector<MaterialGPU>& totalMaterials
)
{
    totalVertexCount      = 0;
    totalIndexCount       = 0;
    int baseInstanceCount = 0;

    for (const MeshComponent* component : meshes)
    {
        const ResourceMesh* resource = component->GetResourceMesh();

        unsigned int vertexCount     = static_cast<unsigned int>(resource->GetVertexCount());
        unsigned int indexCount      = static_cast<unsigned int>(resource->GetIndexCount());

        std::ptrdiff_t idx           = std::distance(uniqueMeshes.begin(), uniqueMeshes.find(resource));
        int accVertexCount           = std::accumulate(
            uniqueMeshesCount.begin(), uniqueMeshesCount.begin() + idx, 0,
            [](int sum, const MeshCount& mesh) { return sum + mesh.vertexCount; }
        );
        int accIndexCount = std::accumulate(
            uniqueMeshesCount.begin(), uniqueMeshesCount.begin() + idx, 0,
            [](int sum, const MeshCount& mesh) { return sum + mesh.indexCount; }
        );

        Command newCommand;
        newCommand.count          = indexCount;        // Number of indices in the mesh
        newCommand.instanceCount  = 1;                 // Number of instances to render
        newCommand.firstIndex     = accIndexCount;     // Index offset in the EBO
        newCommand.baseVertex     = accVertexCount;    // Vertex offset in the VBO
        newCommand.baseInstance   = baseInstanceCount; // Instance Index

        baseInstanceCount        += 1;
        totalVertexCount         += vertexCount;
        totalIndexCount          += indexCount;

        commands.push_back(newCommand);
        totalModels.push_back(component->GetCombinedMatrix());
        totalMaterials.push_back(component->GetResourceMaterial()->GetMaterial());
    }
}