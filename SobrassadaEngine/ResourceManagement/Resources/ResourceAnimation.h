#pragma once
#include "Resource.h"
#include "Math/float3.h"
#include "Math/Quat.h"
#include <unordered_map>

struct Channel
{
    std::vector<float3> positions;
    std::vector<float> posTimeStamps;
    std::vector<Quat> rotations;
    std::vector<float> rotTimeStamps;
    uint32_t numPositions = 0;
    uint32_t numRotations = 0;
};

class ResourceAnimation : public Resource
{
  public:
    ResourceAnimation(UID uid, const std::string& name);
    ~ResourceAnimation() override;

    void SetDuration();
    float GetDuration() { return duration; }
    Channel* GetChannel(const std::string& nodeName);

  public:
    std::unordered_map<std::string, Channel> channels;
    
  private:
    
    float duration = 0.0f;

};
