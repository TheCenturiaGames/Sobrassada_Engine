#pragma once

#include "Resource.h"


class ResourcePrefab : public Resource
{
  public:
    ResourcePrefab(UID uid, const std::string& name);
    ~ResourcePrefab() override;

    // Create and save prefab (JSON format)
    // Load prefab into scene



  private:

};
