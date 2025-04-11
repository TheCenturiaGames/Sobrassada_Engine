#include "MetaFont.h"

MetaFont::MetaFont(UID uid, const std::string& assetPath) : MetaFile(uid, assetPath)
{
}

void MetaFont::AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const
{
}