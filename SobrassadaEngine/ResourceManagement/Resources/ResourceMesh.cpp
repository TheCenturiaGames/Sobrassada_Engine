#include "ResourceMesh.h"

#include "Application.h"
#include "CameraModule.h"
#include "GameObject.h"
#include "OpenGLModule.h"
#include "ResourceMaterial.h"
#include "SceneModule.h"

#include <Math/float2.h>
#include <Math/float4x4.h>
#include <SDL_assert.h>
#include <chrono>
#include <glew.h>
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include "tiny_gltf.h"

ResourceMesh::ResourceMesh(UID uid, const std::string& name, const float3& maxPos, const float3& minPos)
    : Resource(uid, name, ResourceType::Mesh)
{
    aabb.maxPoint = maxPos;
    aabb.minPoint = minPos;
}

ResourceMesh::~ResourceMesh()
{
    if (vao)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }

    if (vbo)
    {
        glDeleteBuffers(1, &vbo);
        vbo = 0;
    }

    if (ebo)
    {
        glDeleteBuffers(1, &ebo);
        ebo = 0;
    }
}

void ResourceMesh::LoadData(
    unsigned int mode, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices
)
{
    this->mode        = mode;
    this->vertexCount = static_cast<unsigned int>(vertices.size());
    this->vertices    = vertices;
    bindPoseVertices  = vertices;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

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

    hasIndices = !indices.empty();
    if (hasIndices)
    {
        this->indexCount = static_cast<unsigned int>(indices.size());
        this->indices    = indices;

        glGenBuffers(1, &ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    }

    // Unbind VAO
    glBindVertexArray(0);
}

void ResourceMesh::Render(
    int program, const float4x4& modelMatrix, unsigned int cameraUBO, const ResourceMaterial* material,
    const std::vector<GameObject*>& bones, const std::vector<float4x4>& bindMatrices
)
{
    auto start = std::chrono::high_resolution_clock::now();

    glUseProgram(program);

    glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
    unsigned int blockIdx = glGetUniformBlockIndex(program, "CameraMatrices");
    glUniformBlockBinding(program, blockIdx, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glUniformMatrix4fv(3, 1, GL_TRUE, modelMatrix.ptr());
    float3 cameraPos;
    if (App->GetSceneModule()->GetMainCamera() != nullptr && App->GetSceneModule()->GetInPlayMode())
        cameraPos = App->GetSceneModule()->GetMainCamera()->GetCameraPosition();
    else cameraPos = App->GetCameraModule()->GetCameraPosition();

    glUniform3fv(glGetUniformLocation(program, "cameraPos"), 1, &cameraPos[0]);

    // CPU Skinning
    if (bones.size() > 0 && bindMatrices.size() > 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        // Vertex* vertices = reinterpret_cast<Vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));
        // for (unsigned int i = 0; i < vertexCount; ++i)
        //{
        //     float4x4 boneInfluence = float4x4::zero;
        //     for (int j = 0; j < 4; ++j)
        //     {
        //         const float4x4& boneTransform  = bones[vertices[i].joint[j]]->GetGlobalTransform();
        //         float4x4 skinSpace             = boneTransform * bindMatrices[vertices[i].joint[j]];
        //         boneInfluence                 += skinSpace * vertices[i].weights[j];
        //     }
        //     vertices[i].position = boneInfluence.MulPos(bindPoseVertices[i].position);
        //
        //     // TODO: rotate normals and tangents
        //     vertices[i].normal   = boneInfluence.MulDir(bindPoseVertices[i].normal);
        //     vertices[i].tangent  = boneInfluence * bindPoseVertices[i].tangent;
        // }
        //
        // glUnmapBuffer(GL_ARRAY_BUFFER);
        // glBindBuffer(GL_ARRAY_BUFFER, 0);

        float4x4 palette[64];
        for (size_t i = 0; i < bones.size(); ++i)
        {
            palette[i] = bones[i]->GetGlobalTransform() * bindMatrices[i];
        }
        glUniformMatrix4fv(glGetUniformLocation(program, "palette"), (GLsizei)bones.size(), GL_TRUE, palette[0].ptr());
        glUniform1i(4, 1); // Tell the shader the mesh has bones
    }
    else
    {
        glUniform1i(4, 0); // Tell the shader the mesh has no bones
    }

    if (material != nullptr)
    {
        material->RenderMaterial(program);
    }

    unsigned int meshTriangles = 0;
    glBindVertexArray(vao);

    if (hasIndices && vao)
    {

        meshTriangles = indexCount / 3;
        App->GetOpenGLModule()->DrawElements(GL_TRIANGLES, static_cast<GLsizei>(indexCount), GL_UNSIGNED_INT, 0);
    }
    else if (vao)
    {
        meshTriangles = vertexCount / 3;
        App->GetOpenGLModule()->DrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertexCount));
    }

    glBindVertexArray(0);

    auto end                             = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = end - start;

    App->GetOpenGLModule()->AddTrianglesPerSecond(meshTriangles / elapsed.count());
    App->GetOpenGLModule()->AddVerticesCount(vertexCount);
}

const float4x4 ResourceMesh::TestSkinning(
    int vertexIndex, const Vertex& vertex, const std::vector<GameObject*>& bones,
    const std::vector<float4x4>& bindMatrices
)
{
    float4x4 boneInfluence = float4x4::zero;
    for (int i = 0; i < 4; ++i)
    {
        const float4x4& boneTransform  = bones[vertex.joint[i]]->GetGlobalTransform();
        float4x4 skinSpace             = boneTransform * bindMatrices[vertex.joint[i]].Inverted();
        boneInfluence                 += skinSpace * vertex.weights[i];
    }

    return boneInfluence;
}
