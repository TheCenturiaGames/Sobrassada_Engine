#include "MetaMesh.h"

MetaMesh::MetaMesh(UID uid, const std::string& assetPath, bool generateTangents)
    : MetaFile(uid, assetPath), generateTangents(generateTangents)
{
}

void MetaMesh::AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const
{
    rapidjson::Value importOptions(rapidjson::kObjectType);
    importOptions.AddMember("generateTangents", generateTangents, allocator);
    doc.AddMember("importOptions", importOptions, allocator);
}