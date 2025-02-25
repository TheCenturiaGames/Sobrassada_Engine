#include "MetaMaterial.h"

MetaMaterial::MetaMaterial(UID uid, const std::string& assetPath, const std::string& shader, bool useAlpha)
    : MetaFile(uid, assetPath), shader(shader), useAlpha(useAlpha)
{
}

void MetaMaterial::AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const
{
    rapidjson::Value importOptions(rapidjson::kObjectType);
    importOptions.AddMember("shader", rapidjson::Value(shader.c_str(), allocator), allocator);
    importOptions.AddMember("useAlpha", useAlpha, allocator);
    doc.AddMember("importOptions", importOptions, allocator);
}