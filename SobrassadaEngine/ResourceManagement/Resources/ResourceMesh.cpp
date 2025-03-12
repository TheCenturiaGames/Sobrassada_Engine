#include "ResourceMesh.h"

#include "Application.h"
#include "CameraModule.h"
#include "OpenGLModule.h"
#include "ResourceMaterial.h"
#include <Math/float2.h>
#include <Math/float4x4.h>
#include <SDL_assert.h>
#include <chrono>
#include <glew.h>

#include "Application.h"
#include "CameraModule.h"
#include "ResourceMaterial.h"

ResourceMesh::ResourceMesh(
    UID uid, const std::string& name, const float3& maxPos, const float3& minPos, const float4x4& transform
)
    : Resource(uid, name, ResourceType::Mesh)
{
    aabb.maxPoint = transform.MulPos(maxPos);
    aabb.minPoint = transform.MulPos(minPos);
}

ResourceMesh::~ResourceMesh()
{
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vao);
}

void ResourceMesh::LoadData(
    unsigned int mode, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices,
    float4x4& transform
)
{
    this->mode              = mode;
    this->vertexCount       = static_cast<unsigned int>(vertices.size());
    this->indexCount        = static_cast<unsigned int>(indices.size());
    this->currentMeshTransform = transform;
    unsigned int bufferSize = sizeof(Vertex);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * bufferSize, vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    glEnableVertexAttribArray(1); // Tangent
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));

    glEnableVertexAttribArray(2); // Normal
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glEnableVertexAttribArray(3); // Texture Coordinates
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    // Unbind VAO
    glBindVertexArray(0);

    this->vertices = vertices;
    this->indices  = indices;
}

void ResourceMesh::Render(int program, const float4x4& modelMatrix, unsigned int cameraUBO, const ResourceMaterial* material) const
{
    auto start = std::chrono::high_resolution_clock::now();

    glUseProgram(program);

    glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
    unsigned int blockIdx = glGetUniformBlockIndex(program, "CameraMatrices");
    glUniformBlockBinding(program, blockIdx, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glUniformMatrix4fv(2, 1, GL_TRUE, modelMatrix.ptr());

    float3 lightDir         = float3(-1.0f, -0.3f, 2.0f);
    float3 lightColor       = float3(1.0f, 1.0f, 1.0f);
    float3 ambientIntensity = float3(1.0f, 1.0f, 1.0f);
    float3 cameraPos        = App->GetCameraModule()->GetCameraPosition();
    glUniform3fv(glGetUniformLocation(program, "cameraPos"), 1, &cameraPos[0]);

    glUniform3fv(glGetUniformLocation(program, "lightDir"), 1, &lightDir[0]);
    glUniform3fv(glGetUniformLocation(program, "lightColor"), 1, &lightColor[0]);

    if (material != nullptr)
    {
        material->RenderMaterial(program);
    }

    unsigned int meshTriangles  = 0;

    if (indexCount > 0 && vao)
    {
        glBindVertexArray(vao);
        meshTriangles   = indexCount / 3;
        App->GetOpenGLModule()->DrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }
    else if (vao)
    {
        glBindVertexArray(vao);
        meshTriangles   = vertexCount / 3;
        App->GetOpenGLModule()->DrawArrays(GL_TRIANGLES, 0, vertexCount);
    }

    glBindVertexArray(0);

    auto end                             = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = end - start;
    
    App->GetOpenGLModule()->AddTrianglesPerSecond(meshTriangles / elapsed.count());
    App->GetOpenGLModule()->AddVerticesCount(vertexCount);
}
