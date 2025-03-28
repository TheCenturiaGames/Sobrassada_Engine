#include "GeometryBatch.h"

#include "Mesh.h"
#include "ResourceMaterial.h"
#include "ResourceMesh.h"
#include "Standalone/MeshComponent.h"

#include "glew.h"
#ifdef _DEBUG
#include "optick.h"
#endif

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
    std::vector<float4x4> totalModels;
    std::vector<MaterialGPU> totalMaterials;

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

void GeometryBatch::Render(const std::vector<MeshComponent*>& meshesToRender)
{
#ifdef _DEBUG
    OPTICK_CATEGORY("GeometryBatch::WaitBuffer", Optick::Category::Wait)
#endif
    WaitBuffer();

#ifdef _DEBUG
    OPTICK_CATEGORY("GeometryBatch::Render", Optick::Category::Rendering)
#endif
    std::vector<Command> commands;
    GenerateCommandsAndSSBO(meshesToRender, commands);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, commands.size() * sizeof(Command), commands.data(), GL_DYNAMIC_DRAW);

    UpdateBuffer(meshesToRender);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, materials);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, materials);

    glBindVertexArray(vao);

    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect);
    glMultiDrawElementsIndirect(
        static_cast<GLenum>(mode), GL_UNSIGNED_INT, (GLvoid*)0, static_cast<GLsizei>(commands.size()), 0
    );

    glBindVertexArray(0);

    LockBuffer();
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

void GeometryBatch::UpdateBuffer(const std::vector<MeshComponent*>& meshesToRender)
{
    int nextBufferIndex  = (currentBufferIndex + 1) % 2;
    GLuint nextBuffer    = models[nextBufferIndex];
    GLuint currentBuffer = models[currentBufferIndex];

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, nextBuffer);

    for (const MeshComponent* component : meshesToRender)
    {
        std::size_t index                 = componentsMap[component];
        ptrModels[nextBufferIndex][index] = component->GetCombinedMatrix();
    }
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