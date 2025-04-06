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

    // Matrix camera configuration
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
    currentPrefab                                 = prefab;

    const std::vector<GameObject*>& prefabObjects = currentPrefab->GetGameObjectsVector();
    if (prefabObjects.empty()) return;

    GameObject* originalRoot = prefabObjects[0];
    previewGO                = CloneGameObjectHierarchy(originalRoot, prefabObjects);

    ApplyInitialTransform();

    if (MeshComponent* mesh = previewGO->GetMeshComponent())
    {
        LoadMeshAndMaterialFromComponent(mesh);
        SetupCamera();
    }
}

// Rebuild the gameobject prefab tree
GameObject* PrefabPortView::CloneGameObjectHierarchy(GameObject* original, const std::vector<GameObject*>& allObjects)
{
    std::unordered_map<UID, GameObject*> cloneMap;

    for (GameObject* go : allObjects)
    {
        GameObject* clone      = new GameObject(*go);
        cloneMap[go->GetUID()] = clone;
    }

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
        App->GetSceneModule()->GetScene()->AddGameObject(clone->GetUID(), clone);
    }
    return cloneMap[original->GetUID()];
}

void PrefabPortView::ApplyInitialTransform()
{
    float4x4 transform =
        float4x4::FromTRS(float3(0, 0, -1.0f), Quat::RotateY(15.0f * DEGREE_RAD_CONV), float3(1, 1, 1));
    previewGO->SetLocalTransform(transform);
    previewGO->UpdateTransformForGOBranch();
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
}

void PrefabPortView::RenderContent()
{
    ImVec2 viewportSize = ImVec2((float)width, (float)height);
    ImVec2 viewportPos  = ImGui::GetCursorScreenPos();

    // Bind the framebuffer and prepare it for rendering
    framebuffer->Bind();
    glViewport(0, 0, width, height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Compute matrices for rendering
    float4x4 model            = previewGO->GetGlobalTransform();
    matrices.viewMatrix       = camera.ViewMatrix();
    matrices.projectionMatrix = camera.ProjectionMatrix();
    matrices.viewMatrix.Transpose();
    matrices.projectionMatrix.Transpose();

    // Upload camera matrices to the uniform buffer
    glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraMatrices), &matrices);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Render the mesh with material (if available)
    int prefabShader = App->GetShaderModule()->GetPrefabProgram();

    loadedMesh->RenderSimple(prefabShader, model, cameraUBO, loadedMaterial);


    framebuffer->Unbind();

    // Draw the rendered image to the ImGui viewport
    ImGui::Image((ImTextureID)(intptr_t)framebuffer->GetTextureID(), viewportSize, ImVec2(0, 1), ImVec2(1, 0));
}


// Where the prefab will be shown
void PrefabPortView::SetupFramebuffer()
{
    framebuffer = new Framebuffer(width, height, true);
}

void PrefabPortView::SetupCamera()
{
    AABB combinedAABB;
    combinedAABB.SetNegativeInfinity();

    for (auto* go : currentPrefab->GetGameObjectsVector())
    {
        combinedAABB.Enclose(go->GetGlobalAABB());
    }

    float3 center = combinedAABB.CenterPoint();
    float size    = (combinedAABB.maxPoint - combinedAABB.minPoint).Length();
    if (size == 0.0f) size = 1.0f;

    float3 pos               = center + float3(0, 0, size * 2.0f);

    camera.type              = FrustumType::PerspectiveFrustum;
    camera.pos               = pos;
    camera.front             = (center - pos).Normalized();
    camera.up                = float3::unitY;
    camera.nearPlaneDistance = 0.1f;
    camera.farPlaneDistance  = 1000.0f;
    camera.horizontalFov     = 60.0f * DEGREE_RAD_CONV;

    float aspect             = (float)framebuffer->GetTextureWidth() / (float)framebuffer->GetTextureHeight();
    camera.verticalFov       = 2.0f * atanf(tanf(camera.horizontalFov * 0.5f) * (1.0f / aspect));
}

// Clean last prefab portview shown and frees resources
void PrefabPortView::CleanupPreview()
{
    if (previewGO)
    {
        UID previewRootUID = previewGO->GetUID();
        App->GetSceneModule()->GetScene()->RemoveGameObjectHierarchy(previewRootUID);
        previewGO = nullptr;
    }

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
