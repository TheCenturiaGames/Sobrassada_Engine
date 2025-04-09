#pragma once

#include "Resource.h"

class ResourceFont : public Resource
{
  public:
    ResourceFont(UID uid, const std::string& name);
    ~ResourceFont() override;

    const std::string& GetFilepath() const { return filepath; }
    void SetFilepath(const std::string& filepath) { this->filepath = filepath; }

  private:
    std::string filepath;
};