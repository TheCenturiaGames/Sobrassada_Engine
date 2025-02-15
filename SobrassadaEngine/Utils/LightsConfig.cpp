#include "LightsConfig.h"

#include "Application.h"
#include "CameraModule.h"
#include "OpenGLModule.h"
#include "ShaderModule.h"
#include "TextureModuleTest.h"
#include "EditorUIModule.h"
#include "LibraryModule.h"
#include "imgui.h"

#include "../Scene/Components/Standalone/Lights/DirectionalLight.h"
#include "../Scene/Components/Standalone/Lights/PointLight.h"
#include "../Scene/Components/Standalone/Lights/SpotLight.h"
#include "glew.h"

LightsConfig::LightsConfig()
{
    skyboxTexture    = 0;
    skyboxVao        = 0;
    skyboxProgram    = 0;
    ambientColor     = float3(1.0f, 1.0f, 1.0f);
    ambientIntensity = 0.2f;
}

LightsConfig::~LightsConfig()
{
    glDeleteBuffers(1, &ambientBufferId);
    glDeleteBuffers(1, &directionalBufferId);
    glDeleteBuffers(1, &pointBufferId);
    glDeleteBuffers(1, &spotBufferId);
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

    // Generate VBO
    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    // Generato VAO
    glGenVertexArrays(1, &skyboxVao);
    glBindVertexArray(skyboxVao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindVertexArray(0);

    skyboxTexture = LoadSkyboxTexture("Test/cubemap.dds");

    // Load the skybox shaders
    skyboxProgram = App->GetShaderModule()->GetProgram("Test/skyboxVertex.glsl", "Test/skyboxFragment.glsl");
}

void LightsConfig::RenderSkybox() const
{
    App->GetOpenGLModule()->SetDepthFunc(false);

    auto projection = App->GetCameraModule()->GetProjectionMatrix();
    auto view       = App->GetCameraModule()->GetViewMatrix();

    glUseProgram(skyboxProgram);
    glUniformMatrix4fv(0, 1, GL_TRUE, &projection[0][0]);
    glUniformMatrix4fv(1, 1, GL_TRUE, &view[0][0]);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);

    glBindVertexArray(skyboxVao);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindVertexArray(0);

    App->GetOpenGLModule()->SetDepthFunc(true);
}

unsigned int LightsConfig::LoadSkyboxTexture(const char* filename) const
{
    std::string stringPath         = std::string(filename);
    std::wstring widePath          = std::wstring(stringPath.begin(), stringPath.end());
    const wchar_t* wideTexturePath = widePath.c_str();
    return App->GetTextureModuleTest()->LoadCubemap(wideTexturePath);
    delete[] wideTexturePath;
}
void LightsConfig::EditorParams()
{
    ImGui::Begin("Lights Config");

    // TODO Skybox texture
    ImGui::SeparatorText("Skybox texture");
    ImGui::Text("TODO Current texture name");
    ImGui::SameLine();
    if (ImGui::Button("Select texture"))
    {
        ImGui::OpenPopup(CONSTANT_TEXTURE_SELECT_DIALOG_ID);
    }

    if (ImGui::IsPopupOpen(CONSTANT_TEXTURE_SELECT_DIALOG_ID))
    {
        App->GetEditorUIModule()->RenderResourceSelectDialog(
            CONSTANT_TEXTURE_SELECT_DIALOG_ID, App->GetLibraryModule()->GetTextureMap()
        );
    }

    ImGui::SeparatorText("Ambient light");
    ImGui::SliderFloat3("Ambient color", &ambientColor[0], 0, 1);
    ImGui::SliderFloat("Ambient intensity", &ambientIntensity, 0, 1);

    ImGui::End();
}

void LightsConfig::InitLightBuffers()
{
    glGenBuffers(1, &ambientBufferId);

    // Buffer for the Directional Light
    glGenBuffers(1, &directionalBufferId);
    glBindBuffer(GL_UNIFORM_BUFFER, directionalBufferId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Lights::DirectionalLightShaderData), nullptr, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 7, directionalBufferId);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Point lights buffer
    glGenBuffers(1, &pointBufferId);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointBufferId);
    int bufferSize = sizeof(Lights::PointLightShaderData) * pointLights.size() + 16;
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);

    // Spot lights buffer
    glGenBuffers(1, &spotBufferId);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, spotBufferId);
    bufferSize =
        (sizeof(Lights::SpotLightShaderData) + 12) * spotLights.size() + 16; // 12 bytes offset between spotlights
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);
}

void LightsConfig::SetLightsShaderData() const
{
    // Ambient light
    Lights::AmbientLightShaderData ambient = Lights::AmbientLightShaderData(float4(ambientColor, ambientIntensity));

    glBindBuffer(GL_UNIFORM_BUFFER, ambientBufferId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ambient), &ambient, GL_STATIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 4, ambientBufferId);

    SetDirectionalLightShaderData();
    SetPointLightsShaderData();
    SetSpotLightsShaderData();
}

void LightsConfig::SetDirectionalLightShaderData() const
{
    if (directionalLight)
    {
        Lights::DirectionalLightShaderData dirLightData(
            float3(directionalLight->GetDirection()),
            float4(directionalLight->GetColor(), directionalLight->GetIntensity())
        );

        glBindBuffer(GL_UNIFORM_BUFFER, directionalBufferId);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Lights::DirectionalLightShaderData), &dirLightData);
        glBindBuffer(GL_UNIFORM_BUFFER, 0);
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
            points.emplace_back(Lights::PointLightShaderData(float4(pointLights[i]->GetGlobalTransform().position, pointLights[i]->GetRange()), float4(pointLights[i]->GetColor(), pointLights[i]->GetIntensity())));
        }
    }

    // This only works whith a constant number of lights. If a new light is added, the buffer must be resized
    int count = points.size();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointBufferId);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int), &count);
    int offset = 16; // Byte start offset for the point light array in the SSBO
    for (const Lights::PointLightShaderData& light : points)
    {
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, sizeof(Lights::PointLightShaderData), &light);
        offset += sizeof(Lights::PointLightShaderData);
    }
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, pointBufferId);
}

void LightsConfig::SetSpotLightsShaderData() const
{
    // This could probably include only lights that changed
    std::vector<Lights::SpotLightShaderData> spots;
    for (int i = 0; i < spotLights.size(); ++i)
    {
        // Fill struct data
        spots.emplace_back(Lights::SpotLightShaderData(
            float4(spotLights[i]->GetGlobalTransform().position, spotLights[i]->GetRange()),
            float4(spotLights[i]->GetColor(), spotLights[i]->GetIntensity()), float3(spotLights[i]->GetDirection()),
            spotLights[i]->GetInnerAngle(), spotLights[i]->GetOuterAngle()
        ));
    }

    // This only works whith a constant number of lights. If a new light is added, the buffer must be resized
    int count = spots.size();
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, spotBufferId);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(int), &count);
    int offset = 16; // Byte start offset for the point light array in the SSBO
    for (const Lights::SpotLightShaderData& light : spots)
    {
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, sizeof(Lights::SpotLightShaderData), &light);
        offset += sizeof(Lights::SpotLightShaderData) + 12;
    }
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, spotBufferId);
}

void LightsConfig::AddDirectionalLight(DirectionalLight* newDirectional)
{
    if (directionalLight == nullptr) directionalLight = newDirectional;
}
void LightsConfig::AddPointLight(PointLight* newPoint)
{
    // Add point light to vector and resize buffer
    pointLights.push_back(newPoint);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointBufferId);
    int bufferSize = sizeof(Lights::PointLightShaderData) * pointLights.size() + 16;
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);

    GLOG(
        "Add point light with uid: %d. Point lights count: %d. Buffer size: %d", newPoint->GetUID(), pointLights.size(),
        bufferSize
    );
}
void LightsConfig::AddSpotLight(SpotLight* newSpot)
{
    spotLights.push_back(newSpot);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, spotBufferId);
    int bufferSize =
        (sizeof(Lights::SpotLightShaderData) + 12) * spotLights.size() + 16; // 12 bytes offset between spotlights
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);

    GLOG(
        "Add spot light with uid: %d. Spot lights count: %d. Buffer size: %d", newSpot->GetUID(), spotLights.size(),
        bufferSize
    );
}

void LightsConfig::RemoveDirectionalLight()
{
    if (directionalLight != nullptr) directionalLight = nullptr;
}

void LightsConfig::RemovePointLight(UID pointUid)
{
    GLOG("Remove point light with UID: %d", pointUid);
    for (int i = 0; i < pointLights.size(); ++i)
    {
        //NO HO TROBA MAI PERQUE ES NULLPTR
        if (pointLights[i]->GetUID() == pointUid)
        {
            // Not optimal to remove an element which is not last from a vector, but this will not happen often
            GLOG("Remove point light in index: %d", i);
            pointLights.erase(pointLights.begin() + i);
            // No need to delete the pointer, because this function is triggered by the destructor and will be deleted afterwards
        }
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, pointBufferId);
    int bufferSize = sizeof(Lights::PointLightShaderData) * pointLights.size() + 16;
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);

    GLOG("Point lights size: %d. Buffer size: %d", pointLights.size(), bufferSize);

}
void LightsConfig::RemoveSpotLight(UID spotUid)
{
    GLOG("Remove spot light with UID: %d", spotUid);
    for (int i = 0; i < spotLights.size(); ++i)
    {
        if (spotLights[i]->GetUID() == spotUid)
        {
            // Not optimal to remove an element which is not last from a vector, but this will not happen often
            GLOG("Remove spot light in index: %d", i);
            spotLights.erase(spotLights.begin() + i);
            // No need to delete the pointer, because this function is triggered by the destructor and will be deleted afterwards

        }
    }

    // Resize lights buffer
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, spotBufferId);
    int bufferSize =
        (sizeof(Lights::SpotLightShaderData) + 12) * spotLights.size() + 16; // 12 bytes offset between spotlights
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, nullptr, GL_STATIC_DRAW);

    GLOG("Spot lights size: %d. Buffer size: %d", spotLights.size(), bufferSize);
}