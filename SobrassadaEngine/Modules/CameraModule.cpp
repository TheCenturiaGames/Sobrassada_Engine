#include "CameraModule.h"

#include "Application.h"
#include "GameObject.h"
#include "InputModule.h"
#include "SceneModule.h"
#include "WindowModule.h"

#include "DebugDraw/debugdraw.h"
#include "Math/Quat.h"
#include "MathGeoLib.h"
#include "SDL_scancode.h"
#include "glew.h"

#include <functional>
#include <tuple>

CameraModule::CameraModule()
{
}

CameraModule::~CameraModule()
{
}

bool CameraModule::Init()
{
    camera.type                      = FrustumType::PerspectiveFrustum;

    camera.pos                       = float3(0, 1, 5);
    camera.front                     = -float3::unitZ;
    camera.up                        = float3::unitY;

    camera.nearPlaneDistance         = 0.1f;
    camera.farPlaneDistance          = 100.f;

    camera.horizontalFov             = (float)HFOV / RAD_DEGREE_CONV;

    int width                        = App->GetWindowModule()->GetWidth();
    int height                       = App->GetWindowModule()->GetHeight();

    camera.verticalFov               = 2.0f * atanf(tanf(camera.horizontalFov * 0.5f) * ((float)height / (float)width));

    viewMatrix                       = camera.ViewMatrix();
    projectionMatrix                 = camera.ProjectionMatrix();

    detachedCamera.type              = FrustumType::PerspectiveFrustum;

    detachedCamera.pos               = float3(0, 1, 5);
    detachedCamera.front             = -float3::unitZ;
    detachedCamera.up                = float3::unitY;
    detachedCamera.nearPlaneDistance = 0.1f;
    detachedCamera.farPlaneDistance  = 100.f;
    detachedCamera.horizontalFov     = (float)HFOV / RAD_DEGREE_CONV;
    camera.verticalFov               = 2.0f * atanf(tanf(camera.horizontalFov * 0.5f) * ((float)height / (float)width));

    detachedViewMatrix               = detachedCamera.ViewMatrix();
    detachedProjectionMatrix         = detachedCamera.ProjectionMatrix();

    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraMatrices), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    lastCastedRay = LineSegment(float3::zero, float3::zero);

    UpdateUBO();

    return true;
}

void CameraModule::UpdateUBO()
{

    matrices.projectionMatrix = GetProjectionMatrix();
    matrices.viewMatrix       = GetViewMatrix();

    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraMatrices), &matrices);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

const LineSegment& CameraModule::CastCameraRay()
{
    auto& windowPosition = App->GetSceneModule()->GetWindowPosition();
    auto& windowSize     = App->GetSceneModule()->GetWindowSize();
    auto& mousePos       = App->GetSceneModule()->GetMousePosition();

    float windowMinX     = std::get<0>(windowPosition);
    float windowMaxX     = std::get<0>(windowPosition) + std::get<0>(windowSize);

    float windowMinY     = std::get<1>(windowPosition);
    float windowMaxY     = std::get<1>(windowPosition) + std::get<1>(windowSize);

    float percentageX    = (std::get<0>(mousePos) - windowMinX) / (windowMaxX - windowMinX);
    float percentageY    = (std::get<1>(mousePos) - windowMinY) / (windowMaxY - windowMinY);

    float normalizedX    = Lerp(-1, 1, percentageX);
    float normalizedY    = Lerp(1, -1, percentageY);

    LineSegment ray;

    if (isCameraDetached) ray = detachedCamera.UnProjectLineSegment(normalizedX, normalizedY);
    else ray = camera.UnProjectLineSegment(normalizedX, normalizedY);
    lastCastedRay = ray;

    return lastCastedRay;
}

update_status CameraModule::Update(float deltaTime)
{
    if (App->GetSceneModule()->GetDoInputs()) Controls(deltaTime);

    return UPDATE_CONTINUE;
}

bool CameraModule::ShutDown()
{
    glDeleteBuffers(1, &ubo);
    return true;
}

void CameraModule::Controls(float deltaTime)
{
    InputModule* inputModule     = App->GetInputModule();
    const KeyState* keyboard     = inputModule->GetKeyboard();
    const KeyState* mouseButtons = inputModule->GetMouseButtons();
    const float2& mouseMotion    = inputModule->GetMouseMotion();

    float finalCameraSpeed       = cameraMoveSpeed * deltaTime;

    // FOCUS & DETACH
    if (keyboard[SDL_SCANCODE_F] == KeyState::KEY_DOWN) TriggerFocusCamera();
    if (keyboard[SDL_SCANCODE_O] == KeyState::KEY_DOWN) ToggleDetachedCamera();

    if (keyboard[SDL_SCANCODE_LSHIFT])
    {
        finalCameraSpeed *= 2;
    }

    if (mouseButtons[SDL_BUTTON_RIGHT - 1])
    {
        // TRANSLATION
        if (keyboard[SDL_SCANCODE_W])
        {
            if (isCameraDetached) detachedCamera.pos += detachedCamera.front * finalCameraSpeed;
            else camera.pos += camera.front * finalCameraSpeed;
        }
        if (keyboard[SDL_SCANCODE_S])
        {
            if (isCameraDetached) detachedCamera.pos -= detachedCamera.front * finalCameraSpeed;
            else camera.pos -= camera.front * finalCameraSpeed;
        }

        if (keyboard[SDL_SCANCODE_A])
        {
            if (isCameraDetached) detachedCamera.pos -= detachedCamera.WorldRight() * finalCameraSpeed;
            else camera.pos -= camera.WorldRight() * finalCameraSpeed;
        }
        if (keyboard[SDL_SCANCODE_D])
        {
            if (isCameraDetached) detachedCamera.pos += detachedCamera.WorldRight() * finalCameraSpeed;
            else camera.pos += camera.WorldRight() * finalCameraSpeed;
        }

        if (keyboard[SDL_SCANCODE_E])
        {
            if (isCameraDetached) detachedCamera.pos += detachedCamera.up * finalCameraSpeed;
            else camera.pos += camera.up * finalCameraSpeed;
        }
        if (keyboard[SDL_SCANCODE_Q])
        {
            if (isCameraDetached) detachedCamera.pos -= detachedCamera.up * finalCameraSpeed;
            else camera.pos -= camera.up * finalCameraSpeed;
        }
    }

    if (mouseButtons[SDL_BUTTON_RIGHT - 1])
    {
        // ZOOMING
        if (keyboard[SDL_SCANCODE_LALT])
        {
            float mouseY = -mouseMotion.y;

            if (mouseY != 0)
            {
                if (isCameraDetached) detachedCamera.pos += detachedCamera.front * mouseY * finalCameraSpeed;
                else camera.pos += camera.front * mouseY * finalCameraSpeed;
            }
        }
        else
        {
            // ROTATION WITH MOUSE
            float mouseX             = mouseMotion.x;
            float mouseY             = mouseMotion.y;
            float deltaRotationAngle = cameraRotationAngle * deltaTime;

            RotateCamera(-mouseX * deltaRotationAngle, -mouseY * deltaRotationAngle);
        }
    }

    // ORBIT
    if (mouseButtons[SDL_BUTTON_LEFT - 1] && keyboard[SDL_SCANCODE_LALT])
    {
        float mouseX             = mouseMotion.x;
        float mouseY             = mouseMotion.y;
        float deltaRotationAngle = cameraRotationAngle * deltaTime;

        RotateCamera(-mouseX * deltaRotationAngle, -mouseY * deltaRotationAngle);

        FocusCamera();
    }

    viewMatrix         = camera.ViewMatrix();
    detachedViewMatrix = detachedCamera.ViewMatrix();

    frustumPlanes.UpdateFrustumPlanes(viewMatrix, projectionMatrix);
    UpdateUBO();
}

void CameraModule::SetAspectRatio(float newAspectRatio)
{
    camera.verticalFov         = 2.0f * atanf(tanf(camera.horizontalFov * 0.5f) * newAspectRatio);
    detachedCamera.verticalFov = 2.0f * atanf(tanf(camera.horizontalFov * 0.5f) * newAspectRatio);

    projectionMatrix           = camera.ProjectionMatrix();
    detachedProjectionMatrix   = detachedCamera.ProjectionMatrix();

    frustumPlanes.UpdateFrustumPlanes(viewMatrix, projectionMatrix);
    UpdateUBO();
}

void CameraModule::TriggerFocusCamera()
{
    if (!isCameraDetached) FocusCamera();
}

void CameraModule::ToggleDetachedCamera()
{
    isCameraDetached = !isCameraDetached;

    if (isCameraDetached)
    {
        detachedCamera.pos   = camera.pos;
        detachedCamera.front = camera.front;
        detachedCamera.up    = camera.up;
    }
}

void CameraModule::RotateCamera(float yaw, float pitch)
{
    Quat yawRotation = Quat::RotateY(yaw);

    if (isCameraDetached)
    {
        detachedCamera.front = yawRotation.Mul(detachedCamera.front).Normalized();
        detachedCamera.up    = yawRotation.Mul(detachedCamera.up).Normalized();
    }
    else
    {
        camera.front = yawRotation.Mul(camera.front).Normalized();
        camera.up    = yawRotation.Mul(camera.up).Normalized();
    }

    if (isCameraDetached)
    {
        Quat pitchRotation = Quat::RotateAxisAngle(detachedCamera.WorldRight(), pitch);

        if (camera.front.y < 0.9f || camera.front.y > -0.9f)
        {
            detachedCamera.front = pitchRotation.Mul(detachedCamera.front).Normalized();
            detachedCamera.up    = pitchRotation.Mul(detachedCamera.up).Normalized();
        }
    }
    else
    {
        if ((currentPitchAngle + pitch) > maximumNegativePitch && (currentPitchAngle + pitch) < maximumPositivePitch)
        {
            currentPitchAngle  += pitch;
            Quat pitchRotation  = Quat::RotateAxisAngle(camera.WorldRight(), pitch);
            camera.front        = pitchRotation.Mul(camera.front).Normalized();
            camera.up           = pitchRotation.Mul(camera.up).Normalized();
        }
    }
}

void CameraModule::FocusCamera()
{
    AABB focusedObjectAABB = App->GetSceneModule()->GetSeletedGameObject()->GetGlobalAABB();
    float3 center          = focusedObjectAABB.CenterPoint();

    if (IsNan(center.x))
    {
        GLOG("Center of bounding box is NaN")
        return;
    }

    // IN CASE THE SELECTED OBJECT SET TO IN OR CLAMP VERY SMALL VALUES TO 0 (errors in float operations)
    int distance = (int)(focusedObjectAABB.maxPoint - focusedObjectAABB.minPoint).Length();

    if (distance == 0 || distance == FLOAT_INF || distance == -FLOAT_INF) distance = 1;

    float3 direction   = camera.front.Normalized();
    float3 newPosition = center - direction * (float)distance;

    camera.pos         = newPosition;
    camera.front       = (center - newPosition).Normalized();

    viewMatrix         = camera.ViewMatrix();
}