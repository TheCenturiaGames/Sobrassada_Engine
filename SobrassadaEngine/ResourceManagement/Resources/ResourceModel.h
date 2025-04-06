#pragma once

#include "Model.h"
#include "Resource.h"

class ResourceModel : public Resource
{
  public:
    ResourceModel(UID uid, const std::string& name);
    ~ResourceModel() override;
    UID GetAnimationUID() const { return animUID; }
    const Model& GetModelData() const { return modelData; }
    void SetAnimationUID(UID animationUID) { animUID = animationUID; }
    const std::vector<UID>& GetAllAnimationUIDs() const { return allAnimUIDs; }
    void SetAllAnimationUIDs(std::vector<UID> animUids) { allAnimUIDs = animUids;}
    void SetModelData(const Model&& model) { modelData = model; }

  private:
    Model modelData;
    UID animUID = INVALID_UID;
    std::vector<UID> allAnimUIDs;
};