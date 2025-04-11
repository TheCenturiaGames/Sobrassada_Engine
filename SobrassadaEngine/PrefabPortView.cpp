#include "PrefabPortView.h"
#include "Application.h"
#include "OpenGLModule.h"
#include "ResourcesModule.h"
#include "SceneModule.h"
#include "ShaderModule.h"
#include "Standalone/MeshComponent.h"

#include "Math/Quat.h"
#include "Math/float4x4.h"
#include <Libs/rapidjson/document.h>

PrefabPortView::PrefabPortView()
{
    SetupFramebuffer();

    // Allocate UBO for camera matrices
    glGenBuffers(1, &cameraUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraMatrices), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

PrefabPortView::~PrefabPortView()
{
    CleanupPreview();
    delete framebuffer;
}

void PrefabPortView::SetPrefab(ResourcePrefab* prefab)
{
    CleanupPreview();
    currentPrefab             = prefab;
    GLOG("Prefab carregat a portview: %llu", prefab->GetUID());

    const auto& prefabObjects = currentPrefab->GetGameObjectsVector();
    if (prefabObjects.empty()) return;

    GameObject* originalRoot = prefabObjects[0];
    previewGO                = CloneGameObjectHierarchy(originalRoot, prefabObjects);

    MeshComponent* mesh      = previewGO->GetMeshComponent();
    if (mesh)
    {
        GLOG("Preview mesh UID: %llu", mesh->GetMeshUID());
        GLOG("Preview material UID: %llu", mesh->GetMaterialUID());
    }

    if (!mesh) return;

    LoadMeshAndMaterialFromComponent(mesh);
    UpdatePreviewTransform();
    SetupCamera();
}

void PrefabPortView::UpdatePreviewTransform()
{
    for (GameObject* go : previewObjects)
        go->UpdateTransformForGOBranch();

    float4x4 transform = previewGO->GetLocalTransform();
    float3 pos, scale;
    Quat rot;
    transform.Decompose(pos, rot, scale);

    float3 previewOffset = float3(0, 0, -1.0f);
    Quat previewRot      = Quat::RotateY(15.0f * DEGREE_RAD_CONV);
    float4x4 adjusted    = float4x4::FromTRS(previewOffset, previewRot * rot, scale);

    previewGO->SetLocalTransform(adjusted);
    previewGO->UpdateTransformForGOBranch();
}

void PrefabPortView::RenderContent()
{
    PrepareRenderTarget();
    UpdateCameraMatrices();
    RenderPreviewMesh();
    RenderToImGui();
    HandleGizmo();
}

void PrefabPortView::PrepareRenderTarget()
{
    framebuffer->Bind();
    glEnable(GL_DEPTH_TEST);
    glViewport(0, 0, width, height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void PrefabPortView::UpdateCameraMatrices()
{
    matrices.viewMatrix       = camera.ViewMatrix();
    matrices.projectionMatrix = camera.ProjectionMatrix();

    glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraMatrices), &matrices);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void PrefabPortView::RenderPreviewMesh()
{
    float4x4 transform = previewGO->GetLocalTransform();
    float3 pos, scale;
    Quat rot;
    transform.Decompose(pos, rot, scale);

    float3 offset      = float3(0, 0, -1.0f);
    Quat extraRotation = Quat::RotateY(15.0f * DEGREE_RAD_CONV);
    float4x4 model     = float4x4::FromTRS(offset, extraRotation * rot, scale * 0.05f);

    int shader         = App->GetShaderModule()->GetPrefabProgram();
    loadedMesh->RenderSimple(shader, model, cameraUBO, loadedMaterial);

    framebuffer->Unbind();
}

void PrefabPortView::RenderToImGui()
{
    ImVec2 size = ImVec2((float)width, (float)height);
    ImVec2 pos  = ImGui::GetCursorScreenPos();

    ImGui::Image((ImTextureID)(intptr_t)framebuffer->GetTextureID(), size, ImVec2(0, 1), ImVec2(1, 0));

    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();
    ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);
}

void PrefabPortView::HandleGizmo()
{
    float* view = (float*)matrices.viewMatrix.Transposed().v;
    float* proj = (float*)matrices.projectionMatrix.Transposed().v;
    float matrix[16];
    memcpy(matrix, previewGO->GetLocalTransform().v, sizeof(matrix));

    ImGuizmo::Manipulate(view, proj, (ImGuizmo::OPERATION)gizmoOperation, ImGuizmo::LOCAL, matrix);

    if (ImGuizmo::IsUsing())
    {
        float4x4 newTransform;
        memcpy(newTransform.v, matrix, sizeof(matrix));

        float3 newPos, newScale;
        Quat newRot;
        newTransform.Decompose(newPos, newRot, newScale);
        newRot.Normalize();

        float3 oldPos, oldScale;
        Quat oldRot;
        previewGO->GetLocalTransform().Decompose(oldPos, oldRot, oldScale);

        float3 finalPos   = (gizmoOperation == ImGuizmo::TRANSLATE) ? newPos : oldPos;
        Quat finalRot     = (gizmoOperation == ImGuizmo::ROTATE) ? newRot : oldRot;
        float3 finalScale = (gizmoOperation == ImGuizmo::SCALE) ? newScale : oldScale;

        previewGO->SetLocalTransform(float4x4::FromTRS(finalPos, finalRot, finalScale));
    }
}

GameObject* PrefabPortView::CloneGameObjectHierarchy(GameObject* original, const std::vector<GameObject*>& allObjects)
{
    std::unordered_map<UID, GameObject*> cloneMap;

    // Clone all the gameobjects
    for (GameObject* go : allObjects)
    {
       GameObject* clone = new GameObject(INVALID_UID, go);

        cloneMap[go->GetUID()] = clone;
        previewObjects.push_back(clone);
    }

    // Reasign hierarchy
    for (GameObject* go : allObjects)
    {
        GameObject* clone = cloneMap[go->GetUID()];
        if (go->GetParent() != INVALID_UID)
        {
            auto it = cloneMap.find(go->GetParent());
            if (it != cloneMap.end())
            {
                GameObject* parentClone = it->second;
                clone->SetParent(parentClone->GetUID());
                parentClone->AddGameObject(clone->GetUID());

            }
        }
    }

    return cloneMap[original->GetUID()];
}

void PrefabPortView::LoadMeshAndMaterialFromComponent(MeshComponent* mesh)
{
    if (const ResourceMesh* resMesh = mesh->GetResourceMesh())
    {
        loadedMesh = static_cast<ResourceMesh*>(App->GetResourcesModule()->RequestResource(resMesh->GetUID()));
        if (loadedMesh && loadedMesh->GetVAO() == 0)
        {
            loadedMesh->UploadToVRAM();
        }
    }

    UID materialUID = mesh->GetMaterialUID();
    if (materialUID != INVALID_UID)
    {
        loadedMaterial = static_cast<ResourceMaterial*>(App->GetResourcesModule()->RequestResource(materialUID));
    }
}

void PrefabPortView::SetupFramebuffer()
{
    framebuffer = new Framebuffer(width, height, true);
}

void PrefabPortView::SetupCamera()
{
    AABB combinedAABB;
    combinedAABB.SetNegativeInfinity();

    for (GameObject* go : previewObjects)
    {
        combinedAABB.Enclose(go->GetGlobalAABB());
    }

    float3 center = combinedAABB.CenterPoint();
    float radius  = combinedAABB.HalfSize().Length();
    if (radius == 0.0f) radius = 1.0f;

    float fovY               = 60.0f * DEGREE_RAD_CONV;
    float aspect             = (float)framebuffer->GetTextureWidth() / (float)framebuffer->GetTextureHeight();  
    float distance           = radius / tanf(fovY * 0.5f);

    float3 cameraDir         = float3(0, 0, -1);
    float3 cameraPos         = center - cameraDir * distance;

    camera.type              = FrustumType::PerspectiveFrustum;
    camera.pos               = cameraPos;
    camera.front             = cameraDir;
    camera.up                = float3::unitY;

    camera.horizontalFov     = 2.0f * atanf(tanf(fovY * 0.5f) * aspect);
    camera.verticalFov       = fovY;

    camera.nearPlaneDistance = radius * 0.1f;
    camera.farPlaneDistance  = distance + radius * 2.0f;
}

void PrefabPortView::CleanupPreview()
{
    for (GameObject* go : previewObjects)
    {
        delete go;
    }
    previewObjects.clear();
    previewGO = nullptr;

    if (loadedMesh)
    {
        App->GetResourcesModule()->ReleaseResource(loadedMesh);
        loadedMesh = nullptr;
    }

    if (loadedMaterial)
    {
        App->GetResourcesModule()->ReleaseResource(loadedMaterial);
        loadedMaterial = nullptr;
    }

    currentPrefab = nullptr;
}
