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
}

CameraComponent::~CameraComponent()
{
    glDeleteBuffers(1, &ubo);
}

void CameraComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
}

void CameraComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();

    if (enabled)
    {
        ImGui::SeparatorText("Camera");
        bool isMainCamera = false;
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
    if (App->GetSceneModule()->GetInPlayMode())
    {
        InputModule* inputModule = App->GetInputModule();
        if (inputModule->GetKey(SDL_SCANCODE_W)) camera.pos += camera.front * 2.0f;
        if (inputModule->GetKey(SDL_SCANCODE_S)) camera.pos -= camera.front * 2.0f;
        if (inputModule->GetKey(SDL_SCANCODE_D)) camera.pos += camera.WorldRight() * 2.0f;
        if (inputModule->GetKey(SDL_SCANCODE_A)) camera.pos -= camera.WorldRight() * 2.0f;
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

    if (!enabled || !drawGizmos) return;
    DebugDrawModule* debug = App->GetDebugDrawModule();
    debug->DrawFrustrum(camera.ProjectionMatrix(), camera.ViewMatrix());
}