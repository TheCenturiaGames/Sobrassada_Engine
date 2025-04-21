#include "MetaMesh.h"

#include "Application.h"
#include "LibraryModule.h"

MetaMesh::MetaMesh(
    UID uid, const std::string& assetPath, bool generateTangents, const float4x4& transform, UID defaultMaterialUID,
    uint32_t gltfMeshIndex, uint32_t gltfPrimitiveIndex
)
    : MetaFile(uid, assetPath), generateTangents(generateTangents), transform(transform),
      defaultMaterialUID(defaultMaterialUID), gltfMeshIndex(gltfMeshIndex), gltfPrimitiveIndex(gltfPrimitiveIndex)
{
    
}

void MetaMesh::AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const
{
    rapidjson::Value importOptions(rapidjson::kObjectType);
    importOptions.AddMember("generateTangents", generateTangents, allocator);
    rapidjson::Value transformArray(rapidjson::kArrayType);
    transformArray.PushBack(transform.ptr()[0], allocator)
        .PushBack(transform.ptr()[1], allocator)
        .PushBack(transform.ptr()[2], allocator)
        .PushBack(transform.ptr()[3], allocator)
        .PushBack(transform.ptr()[4], allocator)
        .PushBack(transform.ptr()[5], allocator)
        .PushBack(transform.ptr()[6], allocator)
        .PushBack(transform.ptr()[7], allocator)
        .PushBack(transform.ptr()[8], allocator)
        .PushBack(transform.ptr()[9], allocator)
        .PushBack(transform.ptr()[10], allocator)
        .PushBack(transform.ptr()[11], allocator)
        .PushBack(transform.ptr()[12], allocator)
        .PushBack(transform.ptr()[13], allocator)
        .PushBack(transform.ptr()[14], allocator)
        .PushBack(transform.ptr()[15], allocator);

    importOptions.AddMember("transform", transformArray, allocator);
    importOptions.AddMember("defaultMaterialUID", defaultMaterialUID, allocator);
    importOptions.AddMember("gltfMeshIndex", gltfMeshIndex, allocator);
    importOptions.AddMember("gltfPrimitiveIndex", gltfPrimitiveIndex, allocator);

    doc.AddMember("importOptions", importOptions, allocator);
}