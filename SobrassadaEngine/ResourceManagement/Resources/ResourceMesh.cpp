#include "ResourceMesh.h"

#include "Mesh.h"

#include "Math/float3.h"

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

    if (importOptions.HasMember("defaultMaterialUID") && importOptions["defaultMaterialUID"].IsUint64())
        defaultMaterialUID = importOptions["defaultMaterialUID"].GetUint64();

    else defaultMaterialUID = INVALID_UID;

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

    if (!indices.empty())
    {
        this->indexCount = static_cast<unsigned int>(indices.size());
        this->indices    = indices;
    }
}
