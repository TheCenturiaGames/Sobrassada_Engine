#include "MetaAnimation.h"
MetaAnimation::MetaAnimation(UID uid, const std::string& assetPath) : MetaFile(uid, assetPath)
{
}


void MetaAnimation::AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const
{
    
}
