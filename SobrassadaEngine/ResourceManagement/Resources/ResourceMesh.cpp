#include "ResourceMesh.h"

#include "Application.h"
#include "BatchManager.h"
#include "CameraComponent.h"
#include "CameraModule.h"
#include "GameObject.h"
#include "Mesh.h"
#include "OpenGLModule.h"
#include "ResourceMaterial.h"
#include "SceneModule.h"

#include "Math/float2.h"
#include "Math/float3.h"
#include "Math/float4x4.h"
#include "SDL_assert.h"
#include "glew.h"
#include "tiny_gltf.h"
#include <chrono>

ResourceMesh::ResourceMesh(
    UID uid, const std::string& name, const float3& maxPos, const float3& minPos, const rapidjson::Value& importOptions
)
    : Resource(uid, name, ResourceType::Mesh)
{
    aabb.maxPoint = maxPos;
    aabb.minPoint = minPos;

    if (importOptions.HasMember("generateTangents") && importOptions["generateTangents"].IsBool())
        generateTangents = importOptions["generateTangents"].GetBool();

    if (importOptions.HasMember("transform") && importOptions["transform"].IsArray() &&
        importOptions["transform"].Size() == 16)
    {
        const rapidjson::Value& transformValue = importOptions["transform"];

        defaultTransform                       = float4x4(
            transformValue[0].GetFloat(), transformValue[1].GetFloat(), transformValue[2].GetFloat(),
            transformValue[3].GetFloat(), transformValue[4].GetFloat(), transformValue[5].GetFloat(),
            transformValue[6].GetFloat(), transformValue[7].GetFloat(), transformValue[8].GetFloat(),
            transformValue[9].GetFloat(), transformValue[10].GetFloat(), transformValue[11].GetFloat(),
            transformValue[12].GetFloat(), transformValue[13].GetFloat(), transformValue[14].GetFloat(),
            transformValue[15].GetFloat()
        );

        aabb.maxPoint = defaultTransform.MulPos(maxPos);
        aabb.minPoint = defaultTransform.MulPos(minPos);
    }
}

ResourceMesh::~ResourceMesh()
{
}

void ResourceMesh::LoadData(
    unsigned int mode, const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices
)
{
    this->mode        = mode;
    this->vertexCount = static_cast<unsigned int>(vertices.size());
    this->vertices    = vertices;
    bindPoseVertices  = vertices;

    if (!indices.empty())
    {
        this->indexCount = static_cast<unsigned int>(indices.size());
        this->indices    = indices;
    }
}

void ResourceMesh::Render(
    int program, const float4x4& modelMatrix, unsigned int cameraUBO, const ResourceMaterial* material,
    const std::vector<GameObject*>& bones, const std::vector<float4x4>& bindMatrices
)
{
    if (bones.size() > 0 && bindMatrices.size() > 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        float4x4 palette[64];
        for (size_t i = 0; i < bones.size(); ++i)
        {
            palette[i] = bones[i]->GetGlobalTransform() * bindMatrices[i];
        }
        glUniformMatrix4fv(glGetUniformLocation(program, "palette"), (GLsizei)bones.size(), GL_TRUE, palette[0].ptr());
        glUniform1i(4, 1); // mesh has bones
    }
    else glUniform1i(4, 0); // mesh has no bones
}

// For the prefab rendering portview
void ResourceMesh::RenderSimple(
    int program, const float4x4& modelMatrix, unsigned int cameraUBO, const ResourceMaterial* material
)
{
    glUseProgram(program);

    // Bind camera UBO
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, cameraUBO);

   // Send model matrix
    GLint modelLoc = glGetUniformLocation(program, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_TRUE, modelMatrix.ptr());


    // Load material texture (if available)
    if (material)
    {
        GLuint texID = material->GetDiffuseTextureID();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texID);

        GLint texLoc = glGetUniformLocation(program, "albedoTex");
        glUniform1i(texLoc, 0); // Texture unit 0
    }

    // Draw mesh
    glBindVertexArray(vao);
    glDrawElements(mode, indexCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);

    glUseProgram(0);
}

void ResourceMesh::UploadToVRAM()
{
    if (vao != 0) return;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    glBindVertexArray(0);
}
