#include "MetaStateMachine.h"

MetaStateMachine::MetaStateMachine(UID uid, const std::string& assetPath) : MetaFile(uid, assetPath)
{
}

void MetaStateMachine::AddImportOptions(rapidjson::Document& doc, rapidjson::Document::AllocatorType& allocator) const
{
}