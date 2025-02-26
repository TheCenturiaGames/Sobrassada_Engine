#pragma once

#include "Scene/Components/Component.h"
#include "ResourceManagement/Resources/ResourceMaterial.h"
#include "ResourceManagement/Resources/ResourceMesh.h"

class PrimitiveMesh : public Component
{
  public:
    PrimitiveMesh(UID uid, UID uidParent, UID uidRoot, const Transform& parentGlobalTransform, const int primitiveType);

    void Update() override;
    void Render() override;

  private:
    std::string currentMeshName       = "Not selected";
    ResourceMesh* currentMesh         = nullptr;

    std::string currentTextureName    = "Not selected";
    ResourceMaterial* currentMaterial = nullptr;
};