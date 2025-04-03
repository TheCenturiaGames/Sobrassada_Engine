#pragma once

#include "CameraComponent.h"
#include "Framebuffer.h"
#include "GameObject.h"
#include "Geometry/Frustum.h"
#include "ResourceMaterial.h"
#include "ResourceMesh.h"
#include "ResourcePrefab.h"

#include "imgui.h"
#include <ImGuizmo.h>
#include <glew.h>
#include <vector>

class PrefabPortView
{
  public:
    PrefabPortView();
    ~PrefabPortView();

    void SetPrefab(ResourcePrefab* prefab);
    void Update(float dt);
    void RenderContent();

    unsigned int GetTextureID() const;

  private:
    void SetupFramebuffer();
    void CleanupPreview();
    void SetupCamera();

  private:
    Framebuffer* framebuffer      = nullptr;
    GameObject* previewGO         = nullptr;
    ResourcePrefab* currentPrefab = nullptr;
    Frustum camera;
    CameraMatrices matrices;
    GLuint cameraUBO                 = 0;

    int width                        = 512;
    int height                       = 512;

    ResourceMesh* loadedMesh         = nullptr;
    ResourceMaterial* loadedMaterial = nullptr;

    int gizmoOperation               = ImGuizmo::ROTATE;
};
