#include "MetaTexture.h"

MetaTexture::MetaTexture(UID uid, const std::string& assetPath, int mipmaps)
    : MetaFile(uid, assetPath), mipmaps(mipmaps)
{
}

void MetaTexture::AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const
{
    rapidjson::Value importOptions(rapidjson::kObjectType);
    importOptions.AddMember("mipmaps", mipmaps, allocator);
    doc.AddMember("importOptions", importOptions, allocator);
}