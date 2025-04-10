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

    // < Public interface >
    void SetPrefab(ResourcePrefab* prefab); // Assign a prefab to preview
    void RenderContent();                   // Main render call from ImGui window

    bool HasValidMesh() const { return loadedMesh != nullptr; }

    GameObject* GetPreviewRoot() const { return previewGO; }
    const std::vector<GameObject*>& GetPreviewObjects() const { return previewObjects; }

    void LoadMeshAndMaterialFromComponent(MeshComponent* mesh);

  private:
    // Rendering pipeline
    void PrepareRenderTarget();
    void UpdateCameraMatrices();
    void RenderPreviewMesh();
    void RenderToImGui();
    void HandleGizmo();

    // Prefab loading
    void CleanupPreview(); // Deletes previous objects and releases resources
    GameObject* CloneGameObjectHierarchy(GameObject* original, const std::vector<GameObject*>& allObjects);
    void UpdatePreviewTransform();

    // Framebuffer and camera
    void SetupFramebuffer();
    void SetupCamera();

  private:
    // Scene data 
    GameObject* previewGO = nullptr;
    std::vector<GameObject*> previewObjects;

    ResourcePrefab* currentPrefab    = nullptr;
    ResourceMesh* loadedMesh         = nullptr;
    ResourceMaterial* loadedMaterial = nullptr;
    float4x4 previewModelMatrix;


    // Rendering
    Framebuffer* framebuffer         = nullptr;
    Frustum camera;
    CameraMatrices matrices;
    GLuint cameraUBO   = 0;

    // Viewport configuration
    int width          = 512;
    int height         = 512;

    // Gizmo configuration
    int gizmoOperation = ImGuizmo::ROTATE;
};
