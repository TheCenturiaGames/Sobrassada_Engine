#include "MetaNavmesh.h"

MetaNavmesh::MetaNavmesh(UID uid, const std::string& assetPath) : MetaFile(uid, assetPath)
{
}

void MetaNavmesh::AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const
{
}