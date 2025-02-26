#include "CameraComponent.h"

#include "Application.h"
#include "DebugDrawModule.h"
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
            camera.type = (currentProjection == 1) ? OrthographicFrustum : PerspectiveFrustum;
        }

        
        //TODO: CHANGE FOV AND UPDATE
        if (camera.type == PerspectiveFrustum)
        {
            //if (ImGui::DragFloat("FoV", &camera.fov, 0.1f, 1.0f, 179.0f, "%.1f"))
            //{
                //camera.fov = std::clamp(camera.fov, 1.0f, 179.0f);
            //}
        }
    }
}

void CameraComponent::Update()
{
}

void CameraComponent::Render()
{
}