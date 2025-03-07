#include "ResourceMesh.h"

#include "Application.h"
#include "CameraModule.h"
#include "ResourceMaterial.h"
#include "GameObject.h"

#include <Math/float2.h>
#include <Math/float4x4.h>
#include <SDL_assert.h>
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
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vao);
}

void ResourceMesh::LoadData(
    unsigned int mode, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices
)
{
    this->mode              = mode;
    this->material          = material;
    this->vertexCount       = static_cast<unsigned int>(vertices.size());
    this->indexCount        = static_cast<unsigned int>(indices.size());
    unsigned int bufferSize = sizeof(Vertex);

    // Store the vertices in bind pose
    bindPoseVertices        = vertices;

    // for (auto vertex : vertices)
    //{
    //     GLOG("Joints: %d, %d, %d, %d", vertex.joint[0], vertex.joint[1], vertex.joint[2], vertex.joint[3]);
    //
    //     GLOG("Weights: %f, %f, %f, %f", vertex.weights[0], vertex.weights[1], vertex.weights[2], vertex.weights[3]);
    // }

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

    glEnableVertexAttribArray(2); // Joint
    glVertexAttribPointer(2, 4, GL_UNSIGNED_INT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, joint));

    glEnableVertexAttribArray(3); // Weights
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, weights));

    glEnableVertexAttribArray(4); // Normal
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    glEnableVertexAttribArray(5); // Texture Coordinates
    glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    // Unbind VAO
    glBindVertexArray(0);
}

void ResourceMesh::Render(
    int program, float4x4& modelMatrix, unsigned int cameraUBO, ResourceMaterial* material,
    const std::vector<GameObject*>& bones, const std::vector<float4x4>& bindMatrices
)
{
    glUseProgram(program);

    glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
    unsigned int blockIdx = glGetUniformBlockIndex(program, "CameraMatrices");
    glUniformBlockBinding(program, blockIdx, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    glUniformMatrix4fv(2, 1, GL_TRUE, &modelMatrix[0][0]);

    float3 cameraPos = App->GetCameraModule()->GetCameraPosition();
    glUniform3fv(glGetUniformLocation(program, "cameraPos"), 1, &cameraPos[0]);

    // CPU Skinning
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    Vertex* vertices = reinterpret_cast<Vertex*>(glMapBuffer(GL_ARRAY_BUFFER, GL_READ_WRITE));
    for (int i = 0; i < vertexCount; ++i)
    {
        //TestSkinning(i, vertices[i], bones, bindMatrices);
        // float4x4 boneInfluence = TestSkinning(i, vertices[i], bones, bindMatrices);
        // vertices[i].position = finalTransform.TranslatePart() * bindPoseVertices[vertexIndex].position;
        // rotate normals and tangents
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

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

const float4x4& ResourceMesh::TestSkinning(
    int vertexIndex, const Vertex& vertex, const std::vector<GameObject*>& bones,
    const std::vector<float4x4>& bindMatrices
)
{
    if (bones.size() < 4 || bindMatrices.size() < 4) return float4x4::identity;

    float4x4 boneInfluence = float4x4::identity;
    for (int i = 0; i < 4; ++i)
    {
        const float4x4& boneTransform = bones[vertex.joint[i]]->GetGlobalTransform();

        // bone weights * bone transform * bone inverse bind matrix * position in bind pose
        boneInfluence += boneTransform * bindMatrices[vertex.joint[i]].Inverted() * vertex.weights[i];
    }
    return boneInfluence;
}
