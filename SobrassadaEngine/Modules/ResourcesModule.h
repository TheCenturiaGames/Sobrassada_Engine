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

  private:
    Resource* CreateNewResource(UID uid);

  private:
    //We will need to do a resourceProgram
    int specularGlossinessProgram = -1;
    int metallicRoughnessProgram  = -1;
    std::map<UID, Resource*> resources;
};
