#include "Resource.h"

#ifdef _DEBUG
#include "MemoryLeaks.h"
#endif

Resource::Resource(UID uid, const std::string& name, ResourceType type) : uid(uid), name(name), type(type)
{
}

Resource::~Resource()
{
}