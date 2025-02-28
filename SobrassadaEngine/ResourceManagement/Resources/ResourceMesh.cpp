#include "ResourceMesh.h"

#include "Application.h"
#include "CameraModule.h"
#include "ResourceMaterial.h"

#include <Math/float2.h>
#include <Math/float4x4.h>
#include <SDL_assert.h>
#include <glew.h>


ResourceMesh::ResourceMesh(UID uid, const std::string& name, const float3& maxPos, const float3& minPos)
#include "Application.h"
#include "CameraModule.h"
#include "ResourceMaterial.h"

ResourceMesh::ResourceMesh(UID uid, const std::string& name, const float3& maxPos, const float3& minPos)
    : Resource(uid, name, ResourceType::Mesh)
{
    aabb.maxPoint = transform.MulPos(maxPos);
    aabb.minPoint = transform.MulPos(minPos);
}

ResourceMesh::~ResourceMesh()
{

    this->mode              = mode;
    this->material          = material;
    this->vertexCount       = vertices.size();
    this->indexCount        = indices.size();
    this->currentMeshTransform         = transform;

void ResourceMesh::LoadData(
    unsigned int mode, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices,
    float4x4& transform
)
{
    this->mode = mode;
    this->material = material;
    this->vertexCount       = static_cast<unsigned int>(vertices.size());
    this->indexCount        = static_cast<unsigned int>(indices.size());
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



glVertexAttribPointer(
    3, 4, GL_FLOAT, GL_FALSE, 0,
    (void *)(sizeof(float) * 3 * vertexCount + sizeof(float) * 2 * textureCoordCount +
             sizeof(float) * 3 * normalCoordCount)
);
}

    glBindVertexArray(0);*/
}

void ResourceMesh::Render(int program, float4x4& modelMatrix, unsigned int cameraUBO, ResourceMaterial* material)
{
    glUseProgram(program);

    glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
    unsigned int blockIdx = glGetUniformBlockIndex(program, "CameraMatrices");
    glUniformBlockBinding(program, blockIdx, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glUniformMatrix4fv(2, 1, GL_TRUE, &modelMatrix[0][0]);

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

    if (indexCount > 0 && vao)
    {
        glBindVertexArray(vao);

        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
    }
    else if (vao)
    {
        glBindVertexArray(vao);

        glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    }

    glBindVertexArray(0);
}
