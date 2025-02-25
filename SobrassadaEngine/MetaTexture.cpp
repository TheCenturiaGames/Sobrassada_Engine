#include "MetaTexture.h"

MetaTexture::MetaTexture(UID uid, const std::string& assetPath, bool mipmaps, const std::string& compression)
    : MetaFile(uid, assetPath), mipmaps(mipmaps), compression(compression)
{
}

void MetaTexture::AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const
{
    rapidjson::Value importOptions(rapidjson::kObjectType);
    importOptions.AddMember("mipmaps", mipmaps, allocator);
    importOptions.AddMember("compression", rapidjson::Value(compression.c_str(), allocator), allocator);
    doc.AddMember("importOptions", importOptions, allocator);
}