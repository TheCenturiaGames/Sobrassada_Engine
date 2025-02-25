#include "MetaMesh.h"

MetaMesh::MetaMesh(UID uid, const std::string& assetPath, bool generateTangents, bool optimizeMesh)
    : MetaFile(uid, assetPath), generateTangents(generateTangents), optimizeMesh(optimizeMesh)
{
}

void MetaMesh::AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const
{
    rapidjson::Value importOptions(rapidjson::kObjectType);
    importOptions.AddMember("generateTangents", generateTangents, allocator);
    importOptions.AddMember("optimizeMesh", optimizeMesh, allocator);
    doc.AddMember("importOptions", importOptions, allocator);
}