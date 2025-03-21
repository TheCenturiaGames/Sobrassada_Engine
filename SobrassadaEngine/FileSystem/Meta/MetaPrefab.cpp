#include "MetaPrefab.h"

MetaPrefab::MetaPrefab(UID uid, const std::string& assetPath) : MetaFile(uid, assetPath)
{
}

void MetaPrefab::AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const
{
}