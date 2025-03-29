#pragma once

#include "Model.h"
#include "Resource.h"

class Model;

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