#include "LightsConfig.h"

#include "Application.h"
#include "CameraComponent.h"
#include "CameraModule.h"
#include "EditorUIModule.h"
#include "Framebuffer.h"
#include "LibraryModule.h"
#include "OpenGLModule.h"
#include "ResourcesModule.h"
#include "SceneModule.h"
#include "ShaderModule.h"
#include "Standalone/Lights/DirectionalLightComponent.h"
#include "Standalone/Lights/PointLightComponent.h"
#include "Standalone/Lights/SpotLightComponent.h"
#include "TextureImporter.h"
#include "ResourceTexture.h"

#include "glew.h"
#include "imgui.h"
#include <cstddef>

LightsConfig::LightsConfig()
{
    ambientColor     = float3(1.0f, 1.0f, 1.0f);
    ambientIntensity = 0.2f;
}

LightsConfig::~LightsConfig()
{
    glDeleteBuffers(1, &ambientBufferId);
    glDeleteBuffers(1, &directionalBufferId);
    glDeleteBuffers(1, &pointBufferId);
    glDeleteBuffers(1, &spotBufferId);

    glDeleteVertexArrays(1, &skyboxVao);
    glDeleteBuffers(1, &skyboxVbo);
    glDeleteProgram(skyboxProgram);
    FreeCubemap();
}

void LightsConfig::FreeCubemap() const
{
    glMakeTextureHandleNonResidentARB(skyboxHandle);
    glDeleteTextures(1, &skyboxID);
    glMakeTextureHandleNonResidentARB(irradianceHandle);
    glDeleteTextures(1, &cubemapIrradiance);
    glMakeTextureHandleNonResidentARB(prefilteredEnvironmentMapHandle);
    glDeleteTextures(1, &prefilteredEnvironmentMap);
}

void LightsConfig::InitSkybox()
{
    float skyboxVertices[] = {-1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
                              1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

                              -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
                              -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

                              1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
                              1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

                              -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
                              1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

                              -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
                              1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

                              -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
                              1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};

    // Generato VAO
    glGenVertexArrays(1, &skyboxVao);
    glBindVertexArray(skyboxVao);

    // Generate VBO
    glGenBuffers(1, &skyboxVbo);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindVertexArray(0);

    // default skybox texture
    LoadSkyboxTexture(App->GetLibraryModule()->GetTextureUID("cubemap"));

    App->GetOpenGLModule()->SetDepthFunc(false);

    cubemapIrradiance = CubeMapToTexture(1024, 1024);
    irradianceHandle  = glGetTextureHandleARB(cubemapIrradiance);
    glMakeTextureHandleResidentARB(skyboxHandle);

    prefilteredEnvironmentMap       = PreFilteredEnvironmentMapGeneration(1024, 1024);
    prefilteredEnvironmentMapHandle = glGetTextureHandleARB(prefilteredEnvironmentMap);
    glMakeTextureHandleResidentARB(prefilteredEnvironmentMapHandle);

    environmentBRDF = EnvironmentBRDFGeneration(1024, 1024);
    environmentBRDFHandle = glGetTextureHandleARB(environmentBRDF);
    glMakeTextureHandleResidentARB(environmentBRDFHandle);

    App->GetOpenGLModule()->SetDepthFunc(true);

    glViewport(
        0, 0, App->GetOpenGLModule()->GetFramebuffer()->GetTextureWidth(),
        App->GetOpenGLModule()->GetFramebuffer()->GetTextureHeight()
    );

    // Load the skybox shaders
    skyboxProgram = App->GetShaderModule()->CreateShaderProgram(SKYBOX_VERTEX_SHADER_PATH, SKYBOX_FRAGMENT_SHADER_PATH);
}

void LightsConfig::RenderSkybox() const
{
    App->GetOpenGLModule()->SetDepthFunc(false);
    float4x4 projection;
    float4x4 view;
    bool change = false;
    if (App->GetSceneModule()->GetInPlayMode() && App->GetSceneModule()->GetScene()->GetMainCamera() != nullptr)
    {
        if (App->GetSceneModule()->GetScene()->GetMainCamera()->GetType() == 1)
        {
            // We need to change to perspective as ortographic doesnt support cubemap
            change = true;
            App->GetSceneModule()->GetScene()->GetMainCamera()->ChangeToPerspective();
        }
        projection = App->GetSceneModule()->GetScene()->GetMainCamera()->GetProjectionMatrix();
        view       = App->GetSceneModule()->GetScene()->GetMainCamera()->GetViewMatrix();
    }
    else
    {
        projection = App->GetCameraModule()->GetProjectionMatrix();
        view       = App->GetCameraModule()->GetViewMatrix();
    }

    glUseProgram(skyboxProgram);
    glUniformMatrix4fv(0, 1, GL_TRUE, projection.ptr());
    glUniformMatrix4fv(1, 1, GL_TRUE, view.ptr());

    glUniformHandleui64ARB(glGetUniformLocation(skyboxProgram, "skybox"), skyboxHandle);

    glBindVertexArray(skyboxVao);
    App->GetOpenGLModule()->DrawArrays(GL_TRIANGLES, 0, 36);

    glBindVertexArray(0);

    if (change == true) App->GetSceneModule()->GetScene()->GetMainCamera()->ChangeToOrtographic();

    App->GetOpenGLModule()->SetDepthFunc(true);
}

void LightsConfig::LoadSkyboxTexture(UID resource)
{
    if (resource == INVALID_UID) return;

    if (currentTexture != nullptr && currentTexture->GetUID() == resource) return;

    ResourceTexture* newCubemap = TextureImporter::LoadCubemap(resource);
    if (newCubemap != nullptr)
    {
        FreeCubemap();
        App->GetResourcesModule()->ReleaseResource(currentTexture);
        currentTexture     = newCubemap;
        currentTextureName = currentTexture->GetName();
        skyboxUID          = currentTexture->GetUID();

        skyboxID           = currentTexture->GetTextureID();
        skyboxHandle       = glGetTextureHandleARB(currentTexture->GetTextureID());
        glMakeTextureHandleResidentARB(skyboxHandle);

        App->GetOpenGLModule()->SetDepthFunc(false);

        /*Im not changing cubemaps right now
        cubemapIrradiance = CubeMapToTexture(1024, 1024);
        irradianceHandle  = glGetTextureHandleARB(cubemapIrradiance);
        glMakeTextureHandleResidentARB(skyboxHandle);

        prefilteredEnvironmentMap       = PreFilteredEnvironmentMapGeneration(1024, 1024);
        prefilteredEnvironmentMapHandle = glGetTextureHandleARB(prefilteredEnvironmentMap);
        glMakeTextureHandleResidentARB(prefilteredEnvironmentMapHandle);

        environmentBRDF       = EnvironmentBRDFGeneration(1024, 1024);
        environmentBRDFHandle = glGetTextureHandleARB(environmentBRDF);
        glMakeTextureHandleResidentARB(environmentBRDFHandle);
        */

        App->GetOpenGLModule()->SetDepthFunc(true);

        glViewport(
            0, 0, App->GetOpenGLModule()->GetFramebuffer()->GetTextureWidth(),
            App->GetOpenGLModule()->GetFramebuffer()->GetTextureHeight()
        );
    }
}

unsigned int LightsConfig::CubeMapToTexture(int width, int height)
{
    const float3 front[6] = {float3::unitX,  -float3::unitX, float3::unitY,
                             -float3::unitY, float3::unitZ,  -float3::unitZ};
    float3 up[6] = {-float3::unitY, -float3::unitY, float3::unitZ, -float3::unitZ, -float3::unitY, -float3::unitY};
    Frustum frustum;
    frustum.type              = FrustumType::PerspectiveFrustum;
    frustum.pos               = float3::zero;
    frustum.nearPlaneDistance = 0.1f;
    frustum.farPlaneDistance  = 100.0f;
    frustum.verticalFov       = PI / 2.0f;
    frustum.horizontalFov     = PI / 2.0f;

    unsigned int irradianceProgram =
        App->GetShaderModule()->CreateShaderProgram(SKYBOX_VERTEX_SHADER_PATH, IRRADIANCE_FRAGMENT_SHADER_PATH);
    glUseProgram(irradianceProgram);

    // TODO: Create and Bind Frame Buffer and Create Irradiance Cubemap
    Framebuffer framebuffer(width, height, false);
    framebuffer.Bind();
    glViewport(0, 0, width, height);
    unsigned int irradianceCubemap;
    glGenTextures(1, &irradianceCubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceCubemap);
    for (int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glUniformHandleui64ARB(glGetUniformLocation(irradianceProgram, "skybox"), skyboxHandle);

    for (int i = 0; i < 6; ++i)
    {
        glFramebufferTexture2D(
            GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceCubemap, 0
        );
        frustum.front       = front[i];
        frustum.up          = up[i];

        float4x4 view       = frustum.ViewMatrix();
        float4x4 projection = frustum.ProjectionMatrix();
        // TODO: Draw 2x2x2 Cube using frustum view and projection matrices

        glUniformMatrix4fv(0, 1, GL_TRUE, projection.ptr());
        glUniformMatrix4fv(1, 1, GL_TRUE, view.ptr());

        glBindVertexArray(skyboxVao);
        App->GetOpenGLModule()->DrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
    }
    framebuffer.Unbind();
    glDeleteProgram(irradianceProgram);

    return framebuffer.GetTextureID();
}

unsigned int LightsConfig::PreFilteredEnvironmentMapGeneration(int width, int height)
{
    const float3 front[6] = {float3::unitX,  -float3::unitX, float3::unitY,
                             -float3::unitY, float3::unitZ,  -float3::unitZ};
    float3 up[6] = {-float3::unitY, -float3::unitY, float3::unitZ, -float3::unitZ, -float3::unitY, -float3::unitY};
    Frustum frustum;
    frustum.type              = FrustumType::PerspectiveFrustum;
    frustum.pos               = float3::zero;
    frustum.nearPlaneDistance = 0.1f;
    frustum.farPlaneDistance  = 100.0f;
    frustum.verticalFov       = PI / 2.0f;
    frustum.horizontalFov     = PI / 2.0f;

    Framebuffer framebuffer(width, height, false);
    framebuffer.Bind();

    unsigned int prefilteredtexture;
    glGenTextures(1, &prefilteredtexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, prefilteredtexture);

    for (int i = 0; i < 6; ++i)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
    }

    numMipMaps = int(log(float(width)) / log(2));
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, numMipMaps);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    unsigned int prefilteredProgram =
        App->GetShaderModule()->CreateShaderProgram(SKYBOX_VERTEX_SHADER_PATH, PREFILTERED_FRAGMENT_SHADER_PATH);
    glUseProgram(prefilteredProgram);

    unsigned int roughnessLocation = glGetUniformLocation(prefilteredProgram, "roughness");
    glUniformHandleui64ARB(glGetUniformLocation(prefilteredProgram, "skybox"), skyboxHandle);

    for (int mip = 0; mip < numMipMaps; ++mip)
    {
        float roughness = (float)mip / (float)(numMipMaps - 1);
        glUniform1f(roughnessLocation, roughness);

        int mipWidth  = static_cast<int>(width * std::pow(0.5f, mip));
        int mipHeight = static_cast<int>(height * std::pow(0.5f, mip));
        glViewport(0, 0, mipWidth, mipHeight);

        // Render each cube plane
        for (unsigned int i = 0; i < 6; ++i)
        {
            glFramebufferTexture2D(
                GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilteredtexture, mip
            );
            // TODO: Draw UnitCube using prefiltered environment map shader and roughness

            frustum.front       = front[i];
            frustum.up          = up[i];

            float4x4 view       = frustum.ViewMatrix();
            float4x4 projection = frustum.ProjectionMatrix();

            glUniformMatrix4fv(0, 1, GL_TRUE, projection.ptr());
            glUniformMatrix4fv(1, 1, GL_TRUE, view.ptr());
            glBindVertexArray(skyboxVao);
            App->GetOpenGLModule()->DrawArrays(GL_TRIANGLES, 0, 36);
            glBindVertexArray(0);
        }
    }

    framebuffer.Unbind();
    glDeleteProgram(prefilteredProgram);
    return framebuffer.GetTextureID();
}

unsigned int LightsConfig::EnvironmentBRDFGeneration(int width, int height)
{
    unsigned int environmentBRDFTexture;
    glGenTextures(1, &environmentBRDFTexture);
    glBindTexture(GL_TEXTURE_2D, environmentBRDFTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, width, height, 0, GL_RG, GL_HALF_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned int prefilteredProgram =
        App->GetShaderModule()->CreateShaderProgram(LIGHTS_VERTEX_SHADER_PATH, ENVIRONMENTBRDF_FRAGMENT_SHADER_PATH);
    glUseProgram(prefilteredProgram);

    glViewport(0, 0, width, height);
    App->GetOpenGLModule()->DrawArrays(GL_TRIANGLES, 0, 36);

    glDeleteProgram(prefilteredProgram);

    return environmentBRDFTexture;
}

void LightsConfig::SaveData(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    rapidjson::Value ambientColorArray(rapidjson::kArrayType);
    ambientColorArray.PushBack(ambientColor.x, allocator)
        .PushBack(ambientColor.y, allocator)
        .PushBack(ambientColor.z, allocator);

    // Add to Light
    targetState.AddMember("Ambient Color", ambientColorArray, allocator);
    targetState.AddMember("Ambient Intensity", ambientIntensity, allocator);
    targetState.AddMember("Skybox UID", skyboxUID, allocator);
}

void LightsConfig::LoadData(const rapidjson::Value& lights)
{
    const rapidjson::Value& ambientColorArray = lights["Ambient Color"];
    ambientColor = {ambientColorArray[0].GetFloat(), ambientColorArray[1].GetFloat(), ambientColorArray[2].GetFloat()};
    ambientIntensity = lights["Ambient Intensity"].GetFloat();
    LoadSkyboxTexture(lights["Skybox UID"].GetUint64());
}

void LightsConfig::EditorParams(bool& lightConfig)
{
    if (!ImGui::Begin("Lights Config", &lightConfig))
    {
        ImGui::End();
        return;
    }

    ImGui::SeparatorText("Skybox texture");
    ImGui::Text(currentTextureName.c_str());
    ImGui::SameLine();
    if (ImGui::Button("Select texture"))
    {
        ImGui::OpenPopup(CONSTANT_TEXTURE_SELECT_DIALOG_ID);
    }

    if (ImGui::IsPopupOpen(CONSTANT_TEXTURE_SELECT_DIALOG_ID))
    {
        const UID uid = App->GetEditorUIModule()->RenderResourceSelectDialog<UID>(
            CONSTANT_TEXTURE_SELECT_DIALOG_ID, App->GetLibraryModule()->GetTextureMap(), INVALID_UID
        );

        if (uid != INVALID_UID) LoadSkyboxTexture(uid);
    }

    ImGui::SeparatorText("Ambient light");
    ImGui::SliderFloat3("Ambient color", &ambientColor[0], 0, 1);
    ImGui::SliderFloat("Ambient intensity", &ambientIntensity, 0, 1);

    ImGui::End();
}

void LightsConfig::InitLightBuffers()
{
    // First generate all buffers
    glGenBuffers(1, &ambientBufferId);
    glGenBuffers(1, &directionalBufferId);
    glGenBuffers(1, &pointBufferId);
    glGenBuffers(1, &spotBufferId);

    // Then get all lights an resize each buffer accordingly
    GetAllSceneLights();
}

void LightsConfig::SetLightsShaderData() const
{
    // Ambient light
    // Lights::AmbientLightShaderData ambient =
    // Lights::AmbientLightShaderData(float4(ambientColor, ambientIntensity), irradianceHandle,
    // prefilteredEnvironmentMapHandle, environmentBRDFHandle, numMipMaps);
    Lights::AmbientLightShaderData ambient = Lights::AmbientLightShaderData(
        float4(ambientColor, ambientIntensity), irradianceHandle, prefilteredEnvironmentMapHandle,
        environmentBRDFHandle, numMipMaps
    );

    glBindBuffer(GL_UNIFORM_BUFFER, ambientBufferId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ambient), &ambient, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, ambientBufferId);

    SetDirectionalLightShaderData();
    SetPointLightsShaderData();
    SetSpotLightsShaderData();
}

void LightsConfig::SetDirectionalLightShaderData() const
{
    if (directionalLight)
    {
        Lights::DirectionalLightShaderData dirLightData(
            float4(directionalLight->GetDirection(), 0.0f),
            float4(directionalLight->GetColor(), directionalLight->GetIntensity())
        );

        glBindBuffer(GL_UNIFORM_BUFFER, directionalBufferId);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Lights::DirectionalLightShaderData), &dirLightData);
        glBindBufferBase(GL_UNIFORM_BUFFER, 3, directionalBufferId);
    }
}

void LightsConfig::SetPointLightsShaderData() const
{
    std::vector<Lights::PointLightShaderData> points;
    for (int i = 0; i < pointLights.size(); ++i)
    {
        if (pointLights[i] != nullptr)
        {
            // Fill struct data
            points.emplace_back(Lights::PointLightShaderData(
                float4(pointLights[i]->GetGlobalTransform().TranslatePart(), pointLights[i]->GetRange()),
                float4(pointLights[i]->GetColor(), pointLights[i]->GetIntensity())
            ));
        }
    }

    // This only works whith a constant number of lights. If a new light is added, the buffer must be resized
    int count = static_cast<int>(points.size());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointBufferId);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int), &count);
    int offset = 16; // Byte start offset for the point light array in the SSBO
    for (const Lights::PointLightShaderData& light : points)
    {
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, sizeof(Lights::PointLightShaderData), &light);
        offset += sizeof(Lights::PointLightShaderData);
    }
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, pointBufferId);
}

void LightsConfig::SetSpotLightsShaderData() const
{
    // This could probably include only lights that changed
    std::vector<Lights::SpotLightShaderData> spots;
    for (int i = 0; i < spotLights.size(); ++i)
    {
        // Fill struct data
        spots.emplace_back(Lights::SpotLightShaderData(
            float4(spotLights[i]->GetGlobalTransform().TranslatePart(), spotLights[i]->GetRange()),
            float4(spotLights[i]->GetColor(), spotLights[i]->GetIntensity()), float3(spotLights[i]->GetDirection()),
            spotLights[i]->GetInnerAngle(), spotLights[i]->GetOuterAngle()
        ));
    }

    // This only works whith a constant number of lights. If a new light is added, the buffer must be resized
    int count = static_cast<int>(spots.size());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, spotBufferId);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int), &count);
    int offset = 16; // Byte start offset for the point light array in the SSBO
    for (const Lights::SpotLightShaderData& light : spots)
    {
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, sizeof(Lights::SpotLightShaderData), &light);
        offset += sizeof(Lights::SpotLightShaderData) + 12;
    }
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, spotBufferId);
}

void LightsConfig::AddDirectionalLight(DirectionalLightComponent* newDirectional)
{
    // Check that the gameObject is in the current scene (to avoid including prefab lights)
    if (App->GetSceneModule()->GetScene()->GetGameObjectByUID(newDirectional->GetParentUID()) == nullptr)
    {
        GLOG("The gameObject is not in the current scene, probably a prefab");
        return;
    }

    if (directionalLight == nullptr) directionalLight = newDirectional;
}
void LightsConfig::AddPointLight(PointLightComponent* newPoint)
{
    // Check that the gameObject is in the current scene (to avoid including prefab lights)
    if (App->GetSceneModule()->GetScene()->GetGameObjectByUID(newPoint->GetParentUID()) == nullptr)
    {
        GLOG("The gameObject is not in the current scene, probably a prefab");
        return;
    }

    // Add point light to vector and resize buffer
    pointLights.push_back(newPoint);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointBufferId);
    int bufferSize = static_cast<int>(sizeof(Lights::PointLightShaderData) * pointLights.size() + 16);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);

    GLOG(
        "Add point light with uid: %d. Point lights count: %d. Buffer size: %d", newPoint->GetUID(), pointLights.size(),
        bufferSize
    );
}
void LightsConfig::AddSpotLight(SpotLightComponent* newSpot)
{
    // Check that the gameObject is in the current scene (to avoid including prefab lights)
    if (App->GetSceneModule()->GetScene()->GetGameObjectByUID(newSpot->GetParentUID()) == nullptr)
    {
        GLOG("The gameObject is not in the current scene, probably a prefab");
        return;
    }

    spotLights.push_back(newSpot);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, spotBufferId);
    int bufferSize = static_cast<int>(
        (sizeof(Lights::SpotLightShaderData) + 12) * spotLights.size() + 16
    ); // 12 bytes offset between spotlights
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);

    GLOG(
        "Add spot light with uid: %d. Spot lights count: %d. Buffer size: %d", newSpot->GetUID(), spotLights.size(),
        bufferSize
    );
}

void LightsConfig::RemoveDirectionalLight(DirectionalLightComponent* directional)
{
    // Check that the gameObject is in the current scene (to avoid including prefab lights)
    if (App->GetSceneModule()->GetScene()->GetGameObjectByUID(directional->GetParentUID()) == nullptr)
    {
        GLOG("The gameObject is not in the current scene, probably a prefab");
        return;
    }

    if (directionalLight != nullptr) directionalLight = nullptr;
}

void LightsConfig::RemovePointLight(PointLightComponent* point)
{
    // Check that the gameObject is in the current scene (to avoid including prefab lights)
    if (App->GetSceneModule()->GetScene()->GetGameObjectByUID(point->GetParentUID()) == nullptr)
    {
        GLOG("The gameObject is not in the current scene, probably a prefab");
        return;
    }

    GLOG("Remove point light with UID: %d", point->GetUID());
    for (int i = 0; i < pointLights.size(); ++i)
    {
        if (pointLights[i] == point)
        {
            // Not optimal to remove an element which is not last from a vector, but this will not happen often
            GLOG("Remove point light in index: %d", i);
            pointLights.erase(pointLights.begin() + i);
            // No need to delete the pointer, because this function is triggered by the destructor and will be deleted
            // afterwards
        }
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointBufferId);
    int bufferSize = static_cast<int>(sizeof(Lights::PointLightShaderData) * pointLights.size() + 16);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);

    GLOG("Point lights size: %d. Buffer size: %d", pointLights.size(), bufferSize);
}

void LightsConfig::RemoveSpotLight(SpotLightComponent* spot)
{
    // Check that the gameObject is in the current scene (to avoid including prefab lights)
    if (App->GetSceneModule()->GetScene()->GetGameObjectByUID(spot->GetParentUID()) == nullptr)
    {
        GLOG("The gameObject is not in the current scene, probably a prefab");
        return;
    }

    GLOG("Remove spot light with UID: %d", spot->GetUID());
    for (int i = 0; i < spotLights.size(); ++i)
    {
        if (spotLights[i] == spot)
        {
            // Not optimal to remove an element which is not last from a vector, but this will not happen often
            GLOG("Remove spot light in index: %d", i);
            spotLights.erase(spotLights.begin() + i);
            // No need to delete the pointer, because this function is triggered by the destructor and will be deleted
            // afterwards
        }
    }

    // Resize lights buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, spotBufferId);
    int bufferSize = static_cast<int>(
        (sizeof(Lights::SpotLightShaderData) + 12) * spotLights.size() + 16
    ); // 12 bytes offset between spotlights
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);

    GLOG("Spot lights size: %d. Buffer size: %d", spotLights.size(), bufferSize);
}

void LightsConfig::GetAllSceneLights()
{
    if (App->GetSceneModule()->GetScene() != nullptr)
    {
        const std::vector<Component*>& components = App->GetSceneModule()->GetScene()->GetAllComponents();
        GetDirectionalLight(components);
        GetAllPointLights(components);
        GetAllSpotLights(components);
    }
}

void LightsConfig::GetAllPointLights(const std::vector<Component*>& components)
{
    pointLights.clear();

    // Iterate through all the components and get the point lights
    for (auto& component : components)
    {
        if (component->GetType() == COMPONENT_POINT_LIGHT)
        {
            GLOG("Add point light");
            pointLights.push_back(static_cast<PointLightComponent*>(component));
        }
    }
    GLOG("Point lights count: %d", pointLights.size());
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointBufferId);
    size_t bufferSize = sizeof(Lights::PointLightShaderData) * pointLights.size() + 16;
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);
}

void LightsConfig::GetAllSpotLights(const std::vector<Component*>& components)
{
    spotLights.clear();

    // Iterate through all the components and get the spot lights
    for (auto& component : components)
    {
        if (component->GetType() == COMPONENT_SPOT_LIGHT)
        {
            GLOG("Add spotlight");
            spotLights.push_back(static_cast<SpotLightComponent*>(component));
        }
    }
    GLOG("Spot lights count: %d", spotLights.size());

    // Maybe make function to do this because it's called like 3 times
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, spotBufferId);
    size_t bufferSize =
        (sizeof(Lights::SpotLightShaderData) + 12) * spotLights.size() + 16; // 12 bytes offset between spotlights
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);
}

void LightsConfig::GetDirectionalLight(const std::vector<Component*>& components)
{
    // Iterate through all the components and get the spot lights
    for (const auto& component : components)
    {
        if (component->GetType() == COMPONENT_DIRECTIONAL_LIGHT)
        {
            GLOG("Add directional light");
            directionalLight = static_cast<DirectionalLightComponent*>(component);
            break;
        }
    }

    glBindBuffer(GL_UNIFORM_BUFFER, directionalBufferId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Lights::DirectionalLightShaderData), nullptr, GL_STATIC_DRAW);
}
