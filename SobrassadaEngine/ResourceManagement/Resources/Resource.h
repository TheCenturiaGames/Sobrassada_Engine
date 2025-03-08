#pragma once

#include "Globals.h"

#include <cstdint>
#include <string>

enum class ResourceType
{
    Unknown = 10,
    Texture,
    Material,
    Mesh,
};

class Resource
{
  public:
    Resource(UID uid, const std::string& name, ResourceType type);
    virtual ~Resource();

    void AddReference() { referenceCount++; }
    void RemoveReference() { referenceCount--; }

    UID GetUID() const { return uid; }
    const std::string& GetName() const { return name; }
    ResourceType GetType() const { return type; }
    unsigned int GetReferenceCount() const { return referenceCount; }
    static ResourceType GetResourceTypeForUID(UID uid) { return static_cast<ResourceType>(uid / UID_PREFIX_DIVISOR); }

  protected:
    UID uid = CONSTANT_EMPTY_UID;
    std::string name;
    ResourceType type           = ResourceType::Unknown;
    unsigned int referenceCount = 0;
};
