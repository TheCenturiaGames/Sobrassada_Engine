#include "MetaModel.h"

MetaModel::MetaModel(UID uid, const std::string& assetPath)
    : MetaFile(uid, assetPath)
{
}

void MetaModel::AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const
{

}