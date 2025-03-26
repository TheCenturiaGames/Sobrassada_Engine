#include "GeometryBatch.h"

#include <Application.h>
#include <Globals.h>
#include <Mesh.h>
#include <OpenGLModule.h>
#include <ResourceMaterial.h>
#include <ResourceMesh.h>
#include <ShaderModule.h>
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

GeometryBatch::GeometryBatch(const MeshComponent* component, int id)
    : id(id), mode(component->GetResourceMesh()->GetMode()), totalVertexCount(0), totalIndexCount(0),
      currentBufferIndex(0)
{
    isMetallic = component->GetResourceMaterial()->GetIsMetallicRoughness();
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &indirect);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenBuffers(2, models);
    glGenBuffers(1, &materials);
    gSync[0]     = nullptr;
    gSync[1]     = nullptr;
    ptrModels[0] = nullptr;
    ptrModels[1] = nullptr;

    GLOG("Batch %d created", id);
}

GeometryBatch::~GeometryBatch()
{
    components.clear();
    componentsMap.clear();
    uniqueMeshesMap.clear();
    uniqueMeshesCount.clear();
    totalModels.clear();
    totalMaterials.clear();

    CleanUp();
    glUseProgram(0);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &indirect);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(2, models);
    glDeleteBuffers(1, &materials);
}

void GeometryBatch::CleanUp()
{
    for (int i = 0; i < 2; i++)
    {
        if (gSync[i]) glDeleteSync(gSync[i]);

        if (ptrModels[i]) glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    }
}

void GeometryBatch::LoadData()
{
    std::vector<Vertex> totalVertices;
    std::vector<unsigned int> totalIndices;

    unsigned int accVertexCount = 0;
    unsigned int accIndexCount  = 0;
    for (const auto& component : components)
    {
        const ResourceMesh* resource = component->GetResourceMesh();

        if (uniqueMeshesMap.find(resource) == uniqueMeshesMap.end())
        {
            const std::vector<Vertex>& vertices      = resource->GetLocalVertices();
            const std::vector<unsigned int>& indices = resource->GetIndices();
            totalVertices.insert(totalVertices.end(), vertices.begin(), vertices.end());
            totalIndices.insert(totalIndices.end(), indices.begin(), indices.end());
            uniqueMeshesMap[resource] = uniqueMeshesMap.size();

            AccMeshCount newMeshCount;
            newMeshCount.accVertexCount = accVertexCount;
            newMeshCount.accIndexCount  = accIndexCount;
            uniqueMeshesCount.push_back(newMeshCount);

            accVertexCount += resource->GetVertexCount();
            accIndexCount  += resource->GetIndexCount();
        }
        componentsMap[component] = componentsMap.size();
        totalModels.push_back(component->GetCombinedMatrix());
        totalMaterials.push_back(component->GetResourceMaterial()->GetMaterial());
    }

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, totalVertices.size() * sizeof(Vertex), totalVertices.data(), GL_STATIC_DRAW);

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
        GL_ELEMENT_ARRAY_BUFFER, totalIndices.size() * sizeof(unsigned int), totalIndices.data(), GL_STATIC_DRAW
    );

    glBindVertexArray(0);

    modelsSize       = totalModels.size() * sizeof(float4x4);
    GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT;
    for (int i = 0; i < 2; i++)
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, models[i]);

        glBufferStorage(GL_SHADER_STORAGE_BUFFER, modelsSize, totalModels.data(), flags);
        ptrModels[i] = (float4x4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, modelsSize, flags);

        if (ptrModels[i] == nullptr)
        {
            GLOG("Error mapping ssbo model %d", i);
            return;
        }
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, materials);
    glBufferData(
        GL_SHADER_STORAGE_BUFFER, totalMaterials.size() * sizeof(MaterialGPU), totalMaterials.data(), GL_STATIC_DRAW
    );

    GLOG("Batch %d loaded succesfully", id);
}

void GeometryBatch::Render(
    unsigned int program, unsigned int cameraUBO, float3 cameraPos, const std::vector<MeshComponent*>& meshesToRender
)
{
    const auto start = std::chrono::high_resolution_clock::now();

    WaitBuffer();

    const int meshTriangles = totalVertexCount / 3;

    glUseProgram(program);

    glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
    unsigned int blockIdx = glGetUniformBlockIndex(program, "CameraMatrices");
    glUniformBlockBinding(program, blockIdx, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glUniform3fv(glGetUniformLocation(program, "cameraPos"), 1, &cameraPos[0]);

    glUniform1i(4, 0); // hasBones

    std::vector<Command> commands;
    GenerateCommandsAndSSBO(meshesToRender, commands);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, commands.size() * sizeof(Command), commands.data(), GL_DYNAMIC_DRAW);

    UpdateBuffer();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, materials);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, materials);

    glBindVertexArray(vao);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect);
    glMultiDrawElementsIndirect(
        static_cast<GLenum>(mode), GL_UNSIGNED_INT, (GLvoid*)0, static_cast<GLsizei>(commands.size()), 0
    );

    glBindVertexArray(0);

    LockBuffer();

    const auto end                             = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<float> elapsed = end - start;

    App->GetOpenGLModule()->AddTrianglesPerSecond(meshTriangles / elapsed.count());
    App->GetOpenGLModule()->AddVerticesCount(totalVertexCount);
}

void GeometryBatch::GenerateCommandsAndSSBO(const std::vector<MeshComponent*>& meshes, std::vector<Command>& commands)
{
    totalVertexCount = 0;
    totalIndexCount  = 0;

    for (const MeshComponent* component : meshes)
    {
        const ResourceMesh* resource = component->GetResourceMesh();

        unsigned int vertexCount     = static_cast<unsigned int>(resource->GetVertexCount());
        unsigned int indexCount      = static_cast<unsigned int>(resource->GetIndexCount());

        std::size_t idx              = uniqueMeshesMap[resource];

        Command newCommand;
        newCommand.count          = indexCount;                            // Number of indices in the mesh
        newCommand.instanceCount  = 1;                                     // Number of instances to render
        newCommand.firstIndex     = uniqueMeshesCount[idx].accIndexCount;  // Index offset in the EBO
        newCommand.baseVertex     = uniqueMeshesCount[idx].accVertexCount; // Vertex offset in the VBO
        newCommand.baseInstance   = static_cast<unsigned int>(componentsMap[component]); // Instance Index

        totalVertexCount         += vertexCount;
        totalIndexCount          += indexCount;

        commands.push_back(newCommand);

        totalModels[componentsMap[component]] = component->GetCombinedMatrix();
    }
}

// gpu not writting in display buffer (currentBuffer)
void GeometryBatch::WaitBuffer()
{
    if (gSync[currentBufferIndex])
    {
        while (1)
        {
            GLenum waitReturn = glClientWaitSync(gSync[currentBufferIndex], GL_SYNC_FLUSH_COMMANDS_BIT, 1);
            if (waitReturn == GL_ALREADY_SIGNALED || waitReturn == GL_CONDITION_SATISFIED) return;
        }
    }
}

void GeometryBatch::UpdateBuffer()
{
    int nextBufferIndex  = (currentBufferIndex + 1) % 2;
    GLuint nextBuffer    = models[nextBufferIndex];
    GLuint currentBuffer = models[currentBufferIndex];

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, nextBuffer);
    memcpy(ptrModels[nextBufferIndex], totalModels.data(), modelsSize);
    glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, currentBuffer);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 10, currentBuffer, 0, modelsSize);

    currentBufferIndex = nextBufferIndex;
}

// lock until gpu stops writting (nextBuffer)
void GeometryBatch::LockBuffer()
{
    if (gSync[currentBufferIndex])
    {
        glDeleteSync(gSync[currentBufferIndex]);
    }
    gSync[currentBufferIndex] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
}