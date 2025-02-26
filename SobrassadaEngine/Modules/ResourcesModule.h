#pragma once

#include "Module.h"
#include "ResourceManagement/Resources/Resource.h"

#include <map>

class ResourcesModule : public Module
{
  public:
    ResourcesModule();
    ~ResourcesModule() override;

    bool Init() override;
    bool ShutDown() override;

    Resource* RequestResource(UID uid);
    void ReleaseResource(const Resource* resource);

    int GetProgram() const { return program; }

  private:
    Resource* CreateNewResource(UID uid);

  private:
    int program = -1;
    std::map<UID, Resource*> resources;
};
