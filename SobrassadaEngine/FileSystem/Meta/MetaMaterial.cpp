#include "MetaMaterial.h"

MetaMaterial::MetaMaterial(UID uid, const std::string& assetPath, const std::string& shader, bool useOcclusion)
    : MetaFile(uid, assetPath), shader(shader), useOcclusion(useOcclusion)
{
}

void MetaMaterial::AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const
{
    rapidjson::Value importOptions(rapidjson::kObjectType);
    importOptions.AddMember("shader", rapidjson::Value(shader.c_str(), allocator), allocator);
    importOptions.AddMember("useOcclusion", useOcclusion, allocator);
    doc.AddMember("importOptions", importOptions, allocator);
}