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
