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

// PrefabPortView renders a preview of a prefab using an isolated camera and framebuffer
class PrefabPortView
{
  public:
    PrefabPortView();
    ~PrefabPortView();

    void SetPrefab(ResourcePrefab* prefab); // Assign a prefab to preview
    void RenderContent();                   // Main render function called from ImGui window
    bool HasValidMesh() const { return loadedMesh != nullptr; }

  private:
    // Resource handling
    void CleanupPreview(); // Deletes old GameObject and releases resources
    GameObject* CloneGameObjectHierarchy(GameObject* original, const std::vector<GameObject*>& allObjects);
    void ApplyInitialTransform();
    void LoadMeshAndMaterialFromComponent(MeshComponent* mesh);

    // Camera setup
    void SetupFramebuffer();
    void SetupCamera();

  private:
    // Rendered objects
    Framebuffer* framebuffer         = nullptr;
    GameObject* previewGO            = nullptr;
    ResourcePrefab* currentPrefab    = nullptr;
    ResourceMesh* loadedMesh         = nullptr;
    ResourceMaterial* loadedMaterial = nullptr;

    // Camera and transform data
    Frustum camera;
    CameraMatrices matrices;
    GLuint cameraUBO   = 0;

    // Viewport size
    int width          = 512;
    int height         = 512;

    // Gizmo config
    int gizmoOperation = ImGuizmo::ROTATE;

    std::vector<GameObject*> previewObjects;
};
