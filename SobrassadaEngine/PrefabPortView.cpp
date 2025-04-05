#include "PrefabPortView.h"
#include "Application.h"
#include "OpenGLModule.h"
#include "ResourcesModule.h"
#include "Standalone/MeshComponent.h"

#include "Math/Quat.h"
#include "Math/float4x4.h"
#include <Libs/rapidjson/document.h>

PrefabPortView::PrefabPortView()
{
    SetupFramebuffer();

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
    currentPrefab       = prefab;

    const auto& objects = prefab->GetGameObjectsVector();
    if (objects.empty()) return;

    CreatePreviewGameObject(objects[0]);
    ApplyInitialTransform();

    if (auto* mesh = previewGO->GetMeshComponent())
    {
        LoadMeshAndMaterialFromComponent(mesh);
    }
}

void PrefabPortView::CreatePreviewGameObject(GameObject* original)
{
    previewGO = new GameObject(*original);
    previewGO->UpdateTransformForGOBranch();
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
    }

    UID materialUID = INVALID_UID;
    rapidjson::Document dummyDoc;
    dummyDoc.SetObject();
    rapidjson::Value dummyState(rapidjson::kObjectType);
    mesh->Save(dummyState, dummyDoc.GetAllocator());

    if (dummyState.HasMember("Material"))
    {
        materialUID = dummyState["Material"].GetUint64();
    }

    if (materialUID != INVALID_UID)
    {
        loadedMaterial = static_cast<ResourceMaterial*>(App->GetResourcesModule()->RequestResource(materialUID));
    }
}

void PrefabPortView::Update(float dt)
{
    if (!currentPrefab) return;
    SetupCamera();
}

void PrefabPortView::RenderContent()
{
    if (!framebuffer || !previewGO || !loadedMesh)
    {
        ImGui::Text("Missing resources to render prefab.");
        return;
    }

    SetupCamera();

    ImVec2 viewportSize = ImVec2((float)width, (float)height);
    ImVec2 viewportPos  = ImGui::GetCursorScreenPos();

    framebuffer->Bind();
    glViewport(0, 0, width, height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float4x4 model            = previewGO->GetGlobalTransform();
    matrices.viewMatrix       = camera.ViewMatrix();
    matrices.projectionMatrix = camera.ProjectionMatrix();
    matrices.viewMatrix.Transpose();
    matrices.projectionMatrix.Transpose();

    glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraMatrices), &matrices);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    if (loadedMesh)
    {
        loadedMesh->Render(0, model, cameraUBO, loadedMaterial, {}, {});
    }

    framebuffer->Unbind();

    ImGui::Image((ImTextureID)(intptr_t)framebuffer->GetTextureID(), viewportSize, ImVec2(0, 1), ImVec2(1, 0));
}

void PrefabPortView::SetupFramebuffer()
{
    framebuffer = new Framebuffer(width, height, true);
}

void PrefabPortView::SetupCamera()
{
    if (!currentPrefab) return;

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

void PrefabPortView::CleanupPreview()
{
    if (previewGO)
    {
        delete previewGO;
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
