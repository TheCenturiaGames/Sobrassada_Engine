#include "CameraComponent.h"

#include "Application.h"
#include "DebugDrawModule.h"
#include "Framebuffer.h"
#include "GameObject.h"
#include "InputModule.h"
#include "OpenGLModule.h"
#include "SceneModule.h"

#include "ImGui.h"
#include "glew.h"
#include <vector>

CameraComponent::CameraComponent(UID uid, GameObject* parent) : Component(uid, parent, "Camera", COMPONENT_CAMERA)
{
    float4x4 globalTransform  = GetGlobalTransform();
    camera.type               = FrustumType::PerspectiveFrustum;
    camera.pos                = float3(globalTransform[0][3], globalTransform[1][3], globalTransform[2][3]);
    camera.front              = -float3(globalTransform[0][2], globalTransform[1][2], globalTransform[2][2]);
    camera.up                 = float3(globalTransform[0][1], globalTransform[1][1], globalTransform[2][1]);

    camera.nearPlaneDistance  = perspectiveNearPlane;
    camera.farPlaneDistance   = perspectiveFarPlane;

    camera.horizontalFov      = (float)HFOV * DEGREE_RAD_CONV;

    auto framebuffer          = App->GetOpenGLModule()->GetFramebuffer();
    int width                 = framebuffer->GetTextureWidth();
    int height                = framebuffer->GetTextureHeight();

    camera.verticalFov        = 2.0f * atanf(tanf(camera.horizontalFov * 0.5f) * ((float)height / (float)width));

    matrices.viewMatrix       = camera.ViewMatrix();
    matrices.projectionMatrix = camera.ProjectionMatrix();

    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraMatrices), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    horizontalFov      = camera.horizontalFov;
    verticalFov        = camera.verticalFov;

    orthographicWidth  = 10.0f;
    orthographicHeight = orthographicWidth * ((float)height / (float)width);

    if (App->GetSceneModule()->GetScene()->GetMainCamera() == nullptr)
    {
        isMainCamera = true;
        App->GetSceneModule()->GetScene()->SetMainCamera(this);
    }

    previewFramebuffer = new Framebuffer(previewWidth, previewHeight, true);
}

CameraComponent::CameraComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent)
{
    if (initialState.HasMember("MainCamera"))
    {
        isMainCamera = initialState["MainCamera"].GetBool();
    }
    if (initialState.HasMember("DrawGizmos"))
    {
        drawGizmos = initialState["DrawGizmos"].GetBool();
    }
    if (initialState.HasMember("SeePreview"))
    {
        seePreview = initialState["SeePreview"].GetBool();
    }

    if (initialState.HasMember("CameraType"))
    {
        camera.type = (initialState["CameraType"].GetInt() == 1) ? OrthographicFrustum : PerspectiveFrustum;
    }
    if (initialState.HasMember("CameraPosition"))
    {
        const rapidjson::Value& cameraPosArray = initialState["CameraPosition"];
        camera.pos = {cameraPosArray[0].GetFloat(), cameraPosArray[1].GetFloat(), cameraPosArray[2].GetFloat()};
    }
    if (initialState.HasMember("CameraFront"))
    {
        const rapidjson::Value& cameraFrontArray = initialState["CameraFront"];
        camera.front = {cameraFrontArray[0].GetFloat(), cameraFrontArray[1].GetFloat(), cameraFrontArray[2].GetFloat()};
    }
    if (initialState.HasMember("CameraUp"))
    {
        const rapidjson::Value& cameraUpArray = initialState["CameraUp"];
        camera.up = {cameraUpArray[0].GetFloat(), cameraUpArray[1].GetFloat(), cameraUpArray[2].GetFloat()};
    }

    if (initialState.HasMember("CameraNearPlane"))
    {
        camera.nearPlaneDistance = initialState["CameraNearPlane"].GetFloat();
    }
    if (initialState.HasMember("CameraFarPlane"))
    {
        camera.farPlaneDistance = initialState["CameraFarPlane"].GetFloat();
    }

    auto framebuffer = App->GetOpenGLModule()->GetFramebuffer();
    int width        = framebuffer->GetTextureWidth();
    int height       = framebuffer->GetTextureHeight();

    if (camera.type == OrthographicFrustum)
    {
        if (initialState.HasMember("CameraOrtographicWidth"))
        {
            camera.orthographicWidth = initialState["CameraOrtographicWidth"].GetFloat();
        }
        if (initialState.HasMember("CameraOrtographicHeight"))
        {
            camera.orthographicHeight = initialState["CameraOrtographicHeight"].GetFloat();
        }
        horizontalFov = (float)HFOV * DEGREE_RAD_CONV;
        verticalFov   = 2.0f * atanf(tanf(camera.horizontalFov * 0.5f) * ((float)height / (float)width));
    }
    else
    {
        if (initialState.HasMember("CameraHFOV"))
        {
            camera.horizontalFov = initialState["CameraHFOV"].GetFloat();
        }
        if (initialState.HasMember("CameraVFOV"))
        {
            camera.verticalFov = initialState["CameraVFOV"].GetFloat();
        }

        orthographicWidth  = 10.0f;
        orthographicHeight = orthographicWidth * ((float)height / (float)width);
    }

    matrices.viewMatrix       = camera.ViewMatrix();
    matrices.projectionMatrix = camera.ProjectionMatrix();

    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(CameraMatrices), nullptr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    previewFramebuffer = new Framebuffer(previewWidth, previewHeight, true);
}

CameraComponent::~CameraComponent()
{
    if (App->GetSceneModule()->GetScene()->GetMainCamera() == this)
    {
        App->GetSceneModule()->GetScene()->SetMainCamera(nullptr);
    }
    glDeleteBuffers(1, &ubo);
    delete previewFramebuffer;
    previewFramebuffer = nullptr;
}

void CameraComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);

    targetState.AddMember("MainCamera", isMainCamera, allocator);
    targetState.AddMember("DrawGizmos", drawGizmos, allocator);
    targetState.AddMember("SeePreview", seePreview, allocator);
    targetState.AddMember("CameraType", (camera.type == OrthographicFrustum) ? 1 : 0, allocator);

    rapidjson::Value cameraPos(rapidjson::kArrayType);
    cameraPos.PushBack(camera.pos.x, allocator).PushBack(camera.pos.y, allocator).PushBack(camera.pos.z, allocator);
    targetState.AddMember("CameraPosition", cameraPos, allocator);

    rapidjson::Value cameraFront(rapidjson::kArrayType);
    cameraFront.PushBack(camera.front.x, allocator)
        .PushBack(camera.front.y, allocator)
        .PushBack(camera.front.z, allocator);
    targetState.AddMember("CameraFront", cameraFront, allocator);

    rapidjson::Value cameraUp(rapidjson::kArrayType);
    cameraUp.PushBack(camera.up.x, allocator).PushBack(camera.up.y, allocator).PushBack(camera.up.z, allocator);
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

void CameraComponent::Clone(const Component* other)
{
    if (other->GetType() == ComponentType::COMPONENT_CAMERA)
    {
        const CameraComponent* otherCamera = static_cast<const CameraComponent*>(other);
        enabled                            = otherCamera->enabled;

        camera                             = otherCamera->camera;
        matrices                           = otherCamera->matrices;
        ubo                                = otherCamera->ubo;

        drawGizmos                         = otherCamera->drawGizmos;
        isMainCamera                       = otherCamera->isMainCamera;

        horizontalFov                      = otherCamera->horizontalFov;
        verticalFov                        = otherCamera->verticalFov;
        perspectiveNearPlane               = otherCamera->perspectiveNearPlane;
        perspectiveFarPlane                = otherCamera->perspectiveFarPlane;

        orthographicWidth                  = otherCamera->orthographicWidth;
        orthographicHeight                 = otherCamera->orthographicHeight;
        ortographicNearPlane               = otherCamera->ortographicNearPlane;
        ortographicFarPlane                = otherCamera->ortographicFarPlane;

        firstTime                          = otherCamera->firstTime;
        seePreview                         = otherCamera->seePreview;
    }
    else
    {
        GLOG("It is not possible to clone a component of a different type!");
    }
}

void CameraComponent::RenderEditorInspector()
{
    Component::RenderEditorInspector();

    if (enabled)
    {
        ImGui::SeparatorText("Camera");
        if (App->GetSceneModule()->GetScene()->GetMainCamera() != nullptr)
            isMainCamera = (App->GetSceneModule()->GetScene()->GetMainCamera()->GetUbo() == ubo);
        if (ImGui::Checkbox("Main Camera", &isMainCamera))
        {
            if (isMainCamera)
            {
                App->GetSceneModule()->GetScene()->SetMainCamera(this);
            }
        }
        ImGui::Checkbox("Draw gizmos", &drawGizmos);
        ImGui::Checkbox("See Preview", &seePreview);

        if (ImGui::DragFloat("Near Plane", &camera.nearPlaneDistance, 0.01f, 0.01f, camera.farPlaneDistance, "%.2f"))
        {
            float cameraNear         = std::max(0.01f, camera.nearPlaneDistance);
            cameraNear               = std::min(cameraNear, camera.farPlaneDistance - 0.1f);
            camera.nearPlaneDistance = cameraNear;
        }
        if (ImGui::DragFloat("Far Plane", &camera.farPlaneDistance, 0.1f, camera.nearPlaneDistance, 1000.f, "%.2f"))
        {
            float cameraFar         = std::min(1000.0f, camera.farPlaneDistance);
            cameraFar               = std::max(cameraFar, camera.nearPlaneDistance + 0.1f);
            camera.farPlaneDistance = cameraFar;
        }

        static const char* projectionTypes[] = {"Perspective", "Orthographic"};
        int currentProjection                = (camera.type == OrthographicFrustum) ? 1 : 0;

        if (ImGui::Combo("Projection Type", &currentProjection, projectionTypes, IM_ARRAYSIZE(projectionTypes)))
        {
            if (currentProjection == 1) ChangeToOrtographic();
            else ChangeToPerspective();
        }

        if (camera.type == PerspectiveFrustum)
        {
            float hfov = camera.horizontalFov * RAD_DEGREE_CONV;
            if (ImGui::DragFloat("FoV", &hfov, 0.1f, 1.0f, 179.0f, "%.2f"))
            {
                camera.horizontalFov = hfov * DEGREE_RAD_CONV;
                auto framebuffer     = App->GetOpenGLModule()->GetFramebuffer();
                int width            = framebuffer->GetTextureWidth();
                int height           = framebuffer->GetTextureHeight();
                camera.verticalFov   = 2.0f * atanf(tanf(camera.horizontalFov * 0.5f) * ((float)height / (float)width));
            }
        }
        else if (camera.type == OrthographicFrustum)
        {
            if (ImGui::DragFloat("Width", &camera.orthographicWidth, 0.1f, 0.1f, 200.f, "%.2f"))
            {
                auto framebuffer          = App->GetOpenGLModule()->GetFramebuffer();
                int width                 = framebuffer->GetTextureWidth();
                int height                = framebuffer->GetTextureHeight();
                camera.orthographicHeight = camera.orthographicWidth * ((float)height / (float)width);
            }
        }
    }
}

void CameraComponent::Update(float deltaTime)
{
    if (isMainCamera && App->GetSceneModule()->GetScene()->GetMainCamera() == nullptr)
        App->GetSceneModule()->GetScene()->SetMainCamera(this);

    float4x4 globalTransform = GetGlobalTransform();
    camera.pos               = float3(globalTransform[0][3], globalTransform[1][3], globalTransform[2][3]);
    camera.front     = -float3(globalTransform[0][2], globalTransform[1][2], globalTransform[2][2]).Normalized();
    camera.up        = float3(globalTransform[0][1], globalTransform[1][1], globalTransform[2][1]).Normalized();

    auto framebuffer = App->GetOpenGLModule()->GetFramebuffer();
    int width        = framebuffer->GetTextureWidth();
    int height       = framebuffer->GetTextureHeight();
    SetAspectRatio((float)height / (float)width);

    matrices.projectionMatrix = camera.ProjectionMatrix();
    matrices.viewMatrix       = camera.ViewMatrix();

    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(CameraMatrices), &matrices);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    frustumPlanes.UpdateFrustumPlanes(camera.ViewMatrix(), camera.ProjectionMatrix());

    if (App->GetSceneModule()->GetScene()->GetSelectedGameObject() == parent && seePreview) previewEnabled = true;
    else previewEnabled = false;
}

void CameraComponent::RenderCameraPreview(float deltaTime)
{
    int mainFramebufferWidth  = App->GetOpenGLModule()->GetFramebuffer()->GetTextureWidth();
    int mainFramebufferHeight = App->GetOpenGLModule()->GetFramebuffer()->GetTextureHeight();

    float scaleFactor         = 0.2f;
    previewWidth  = static_cast<int>(mainFramebufferWidth * scaleFactor);
    previewHeight = static_cast<int>(previewWidth / camera.AspectRatio());

    if (!autorendering)
    {
        previewFramebuffer->Resize(previewWidth, previewHeight);
        previewFramebuffer->CheckResize();

        glViewport(0, 0, previewWidth, previewHeight);

        previewFramebuffer->Bind();
        autorendering = true;
        App->GetSceneModule()->GetScene()->RenderScene(deltaTime, this);
        autorendering = false;

        App->GetOpenGLModule()->GetFramebuffer()->Bind();

        glViewport(0, 0, mainFramebufferWidth, mainFramebufferHeight);
    }

    static bool open          = true;
    ImVec2 pos(
        static_cast<float>(mainFramebufferWidth - previewWidth + 200),
        static_cast<float>(mainFramebufferHeight - previewHeight + 50)
    );

    // Set the size of the preview window
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(previewWidth + 16), static_cast<float>(previewHeight + 16)));
    ImGui::SetNextWindowPos(pos);

    if (ImGui::Begin(
            "Camera Preview", &open,
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
        ))
    {
        ImTextureID texID = (ImTextureID)(intptr_t)previewFramebuffer->GetTextureID();
        ImVec2 size((float)previewWidth, (float)previewHeight);
        ImVec2 uv0 = ImVec2(0, 1);
        ImVec2 uv1 = ImVec2(
            (float)previewWidth / previewFramebuffer->GetTextureWidth(),
            1.0f - ((float)previewHeight / previewFramebuffer->GetTextureHeight())
        );
        ImGui::Image(texID, size, uv0, uv1);
    }
    ImGui::End();
}

void CameraComponent::Render(float deltaTime)
{
    if (!enabled || !drawGizmos || App->GetSceneModule()->GetInPlayMode()) return;
    DebugDrawModule* debug = App->GetDebugDrawModule();
    debug->DrawFrustrum(camera.ProjectionMatrix(), camera.ViewMatrix());
    if (previewEnabled) RenderCameraPreview(deltaTime);
}

void CameraComponent::SetAspectRatio(float newAspectRatio)
{
    if (camera.type == PerspectiveFrustum)
    {
        camera.verticalFov = 2.0f * atanf(tanf(camera.horizontalFov * 0.5f) * newAspectRatio);
    }
    if (camera.type == OrthographicFrustum)
    {
        camera.orthographicHeight = camera.orthographicWidth * newAspectRatio;
    }
}

void CameraComponent::ChangeToPerspective()
{
    orthographicWidth        = camera.orthographicWidth;
    orthographicHeight       = camera.orthographicHeight;

    camera.type              = PerspectiveFrustum;
    camera.horizontalFov     = horizontalFov;
    camera.verticalFov       = verticalFov;
    camera.nearPlaneDistance = perspectiveNearPlane;
    camera.farPlaneDistance  = perspectiveFarPlane;
}

void CameraComponent::ChangeToOrtographic()
{
    horizontalFov             = camera.horizontalFov;
    verticalFov               = camera.verticalFov;

    camera.type               = OrthographicFrustum;
    camera.orthographicWidth  = orthographicWidth;
    camera.orthographicHeight = orthographicHeight;
    camera.nearPlaneDistance  = ortographicNearPlane;
    camera.farPlaneDistance   = ortographicFarPlane;
}