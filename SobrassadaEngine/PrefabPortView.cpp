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
    currentPrefab                           = prefab;

    const std::vector<GameObject*>& objects = prefab->GetGameObjectsVector();
    if (!objects.empty())
    {
        previewGO = new GameObject(*objects[0]);
        previewGO->UpdateTransformForGOBranch();

        // Apply default transform so the gizmo appears centered and visible
        Quat rotation = Quat::RotateY(15.0f * DEGREE_RAD_CONV);
        previewGO->SetLocalTransform(float4x4::FromTRS(float3(0, 0, -1.0f), rotation, float3(1, 1, 1)));

        MeshComponent* mesh = previewGO->GetMeshComponent();
        if (mesh)
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
                loadedMaterial =
                    static_cast<ResourceMaterial*>(App->GetResourcesModule()->RequestResource(materialUID));
            }
        }
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

    // Prepare the image rectangle
    ImVec2 viewportSize = ImVec2((float)width, (float)height);
    ImVec2 viewportPos  = ImGui::GetCursorScreenPos();

    ImTextureID texID   = (ImTextureID)(intptr_t)framebuffer->GetTextureID();

    // Draw image
    ImGui::Image(texID, viewportSize, ImVec2(0, 1), ImVec2(1, 0));

    // Set drawlist and rect
    ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());
    ImGuizmo::SetRect(viewportPos.x, viewportPos.y, viewportSize.x, viewportSize.y);
    ImGuizmo::Enable(true);

    // Render to framebuffer
    framebuffer->Bind();
    glViewport(0, 0, width, height);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float4x4 viewMatrix  = camera.ViewMatrix();
    float4x4 projMatrix  = camera.ProjectionMatrix();
    float4x4 modelMatrix = previewGO->GetGlobalTransform();

    CameraMatrices matrices;
    matrices.viewMatrix       = viewMatrix;
    matrices.projectionMatrix = projMatrix;

    glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraMatrices), &matrices);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    int shaderProgram = App->GetShaderModule()->GetMetallicRoughnessProgram();
    if (loadedMaterial && !loadedMaterial->GetIsMetallicRoughness())
    {
        shaderProgram = App->GetShaderModule()->GetSpecularGlossinessProgram();
    }

    loadedMesh->Render(
        shaderProgram, modelMatrix, cameraUBO, loadedMaterial, std::vector<GameObject*>(), std::vector<float4x4>()
    );

    framebuffer->Unbind();

    // Gizmo
    float4x4 model = previewGO->GetLocalTransform();
    ImGuizmo::Manipulate(viewMatrix.ptr(), projMatrix.ptr(), ImGuizmo::ROTATE, ImGuizmo::LOCAL, model.ptr());

    if (ImGuizmo::IsUsing())
    {
        previewGO->SetLocalTransform(model);
        previewGO->UpdateTransformForGOBranch();
    }
}




unsigned int PrefabPortView::GetTextureID() const
{
    return framebuffer ? framebuffer->GetTextureID() : 0;
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

    for (GameObject* go : currentPrefab->GetGameObjectsVector())
    {
        combinedAABB.Enclose(go->GetGlobalAABB());
    }

    float3 aabbCenter = combinedAABB.CenterPoint();
    float aabbSize    = (combinedAABB.maxPoint - combinedAABB.minPoint).Length();
    if (aabbSize == 0.0f) aabbSize = 1.0f;

    float3 cameraPos          = aabbCenter + float3(0.0f, 0.0f, aabbSize * 3.0f); // pushed further back
    float3 cameraTarget       = aabbCenter;
    float3 up                 = float3::unitY;

    camera.type               = FrustumType::PerspectiveFrustum;
    camera.pos                = cameraPos;
    camera.front              = (cameraTarget - cameraPos).Normalized();
    camera.up                 = up;
    camera.nearPlaneDistance  = 0.1f;
    camera.farPlaneDistance   = 1000.0f;
    camera.horizontalFov      = 60.0f * DEGREE_RAD_CONV;

    float aspectRatio         = (float)framebuffer->GetTextureWidth() / (float)framebuffer->GetTextureHeight();
    camera.verticalFov        = 2.0f * atanf(tanf(camera.horizontalFov * 0.5f) * (1.0f / aspectRatio));

    matrices.projectionMatrix = camera.ProjectionMatrix();
    matrices.viewMatrix       = camera.ViewMatrix();

    glBindBuffer(GL_UNIFORM_BUFFER, cameraUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraMatrices), &matrices);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
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
