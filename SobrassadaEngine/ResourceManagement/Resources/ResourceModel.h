#pragma once

#include "Resource.h"
#include "FileSystem/Model.h"

class ResourceModel : public Resource
{
  public:
    ResourceModel(UID uid, const std::string& name);
    ~ResourceModel() override;

    const Model& GetModelData() const { return modelData; }

    void SetModelData(const Model&& model) { modelData = model; }

   private:
    Model modelData;
};