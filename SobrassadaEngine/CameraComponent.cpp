#include "CameraComponent.h"

#include "Application.h"
#include "DebugDrawModule.h"
#include "InputModule.h"
#include "SceneModule.h"
#include "WindowModule.h"
#include "glew.h"

#include "ImGui.h"
#include <vector>

CameraComponent::CameraComponent(UID uid, UID uidParent, UID uidRoot, const Transform& parentGlobalTransform)
    : Component(uid, uidParent, uidRoot, "Camera", COMPONENT_CAMERA, parentGlobalTransform)
{

    camera.type               = FrustumType::PerspectiveFrustum;

    camera.pos                = float3(0, 1, 5);
    camera.front              = -float3::unitZ;
    camera.up                 = float3::unitY;

    camera.nearPlaneDistance  = 0.1f;
    camera.farPlaneDistance   = 100.f;

    camera.horizontalFov      = (float)HFOV * DEGTORAD;

    int width                 = App->GetWindowModule()->GetWidth();
    int height                = App->GetWindowModule()->GetHeight();

    camera.verticalFov        = 2.0f * atanf(tanf(camera.horizontalFov * 0.5f) * ((float)height / (float)width));

    matrices.viewMatrix       = camera.ViewMatrix();
    matrices.projectionMatrix = camera.ProjectionMatrix();

    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraMatrices), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

CameraComponent::CameraComponent(const rapidjson::Value& initialState) : Component(initialState)
{
    if (initialState.HasMember("CameraType"))
    {
        camera.type = (initialState["Type"].GetInt() == 1) ? OrthographicFrustum : PerspectiveFrustum;
    }
    if (initialState.HasMember("CameraPosition"))
    {
        //float 3
        //camera.pos = initialState["Camera position"].GetFloat();
    }

    matrices.viewMatrix       = camera.ViewMatrix();
    matrices.projectionMatrix = camera.ProjectionMatrix();

    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraMatrices), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

CameraComponent::~CameraComponent()
{
    glDeleteBuffers(1, &ubo);
}

void CameraComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);
    
    targetState.AddMember("MainCamera", isMainCamera, allocator);
    targetState.AddMember("CameraType", (camera.type == OrthographicFrustum) ? 1 : 0, allocator);

    rapidjson::Value cameraPos(rapidjson::kArrayType);
    cameraPos.PushBack(camera.pos.x, allocator)
        .PushBack(camera.pos.y, allocator)
        .PushBack(camera.pos.z, allocator);
    targetState.AddMember("CameraPosition", cameraPos, allocator);

    rapidjson::Value cameraFront(rapidjson::kArrayType);
    cameraFront.PushBack(camera.front.x, allocator)
        .PushBack(camera.front.y, allocator)
        .PushBack(camera.front.z, allocator);
    targetState.AddMember("CameraFront", cameraFront, allocator);

    rapidjson::Value cameraUp(rapidjson::kArrayType);
    cameraUp.PushBack(camera.up.x, allocator)
        .PushBack(camera.up.y, allocator)
        .PushBack(camera.up.z, allocator);
    targetState.AddMember("CameraUp", cameraUp, allocator);

    targetState.AddMember("CameraNearPlane", camera.nearPlaneDistance, allocator);
    targetState.AddMember("CameraFarPlane", camera.farPlaneDistance, allocator);

    if (camera.type == OrthographicFrustum)
    {
        targetState.AddMember("CameraOrtographicWidth", camera.orthographicWidth, allocator);
        targetState.AddMember("CameraOrtographicHeight", camera.orthographicHeight, allocator);
    }
    else
    {
        targetState.AddMember("CameraHFOV", camera.horizontalFov, allocator);
        targetState.AddMember("CameraVFOV", camera.verticalFov, allocator);
    }
}

void CameraComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();

    if (enabled)
    {
        ImGui::SeparatorText("Camera");
        if (App->GetSceneModule()->GetMainCamera() != nullptr)
            isMainCamera = (App->GetSceneModule()->GetMainCamera()->GetUbo() == ubo);
        if (ImGui::Checkbox("Main Camera", &isMainCamera))
        {
            if (isMainCamera)
            {
                App->GetSceneModule()->SetMainCamera(this);
            }
        }
        ImGui::Checkbox("Draw gizmos", &drawGizmos);

        if (ImGui::DragFloat("Near Plane", &camera.nearPlaneDistance, 0.01f, 0.01f, camera.farPlaneDistance, "%.2f"))
        {
            float cameraNear         = std::max(0.01f, camera.nearPlaneDistance);
            cameraNear               = std::min(cameraNear, camera.farPlaneDistance);
            camera.nearPlaneDistance = cameraNear;
        }
        if (ImGui::DragFloat("Far Plane", &camera.farPlaneDistance, 0.1f, camera.nearPlaneDistance, 1000.f, "%.2f"))
        {
            float cameraFar         = std::min(1000.0f, camera.farPlaneDistance);
            cameraFar               = std::max(cameraFar, camera.nearPlaneDistance);
            camera.farPlaneDistance = cameraFar;
        }

        static const char* projectionTypes[] = {"Perspective", "Orthographic"};
        int currentProjection                = (camera.type == OrthographicFrustum) ? 1 : 0;

        if (ImGui::Combo("Projection Type", &currentProjection, projectionTypes, IM_ARRAYSIZE(projectionTypes)))
        {
            camera.type               = (currentProjection == 1) ? OrthographicFrustum : PerspectiveFrustum;
            matrices.projectionMatrix = camera.ProjectionMatrix();
        }

        if (camera.type == PerspectiveFrustum)
        {
            float hfov = camera.horizontalFov * RADTODEG;
            if (ImGui::DragFloat("FoV", &hfov, 0.1f, 1.0f, 179.0f, "%.2f"))
            {
                camera.horizontalFov = hfov * DEGTORAD;
                int width            = App->GetWindowModule()->GetWidth();
                int height           = App->GetWindowModule()->GetHeight();
                camera.verticalFov   = 2.0f * atanf(tanf(camera.horizontalFov * 0.5f) * ((float)height / (float)width));
                matrices.projectionMatrix = camera.ProjectionMatrix();
            }
        }
        else if (camera.type == OrthographicFrustum)
        {
            if (ImGui::DragFloat("Width", &camera.orthographicWidth, 0.1f, 0.1f, 1000.f, "%.2f"))
            {
                int width                 = App->GetWindowModule()->GetWidth();
                int height                = App->GetWindowModule()->GetHeight();
                camera.orthographicHeight = camera.orthographicWidth / ((float)height / (float)width);
                matrices.projectionMatrix = camera.ProjectionMatrix();
            }
        }
    }
}

void CameraComponent::Update()
{
    /*Los controles deberian estar en otro script, no aqui (es solo para prueba)*/
    if (App->GetSceneModule()->GetInPlayMode())
    {
        InputModule* inputModule = App->GetInputModule();
        if (inputModule->GetKey(SDL_SCANCODE_W)) camera.pos += camera.front * 1.0f;
        if (inputModule->GetKey(SDL_SCANCODE_S)) camera.pos -= camera.front * 1.0f;
        if (inputModule->GetKey(SDL_SCANCODE_D)) camera.pos += camera.WorldRight() * 1.0f;
        if (inputModule->GetKey(SDL_SCANCODE_A)) camera.pos -= camera.WorldRight() * 1.0f;
    }

    matrices.projectionMatrix = camera.ProjectionMatrix();
    matrices.viewMatrix       = camera.ViewMatrix();

    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraMatrices), &matrices);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    frustumPlanes.UpdateFrustumPlanes(camera.ViewMatrix(), camera.ProjectionMatrix());
}

void CameraComponent::Render()
{

    if (!enabled || !drawGizmos || App->GetSceneModule()->GetInPlayMode()) return;
    DebugDrawModule* debug = App->GetDebugDrawModule();
    debug->DrawFrustrum(camera.ProjectionMatrix(), camera.ViewMatrix());
}