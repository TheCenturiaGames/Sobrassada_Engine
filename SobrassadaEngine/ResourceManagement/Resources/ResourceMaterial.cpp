#include "ResourceMaterial.h"

#include "Application.h"
#include "LibraryModule.h"
#include "Material.h"
#include "ResourceTexture.h"
#include "TextureImporter.h"

#include "glew.h"
#include "imgui.h"
#include <unordered_set>

ResourceMaterial::ResourceMaterial(UID uid, const std::string& name, const rapidjson::Value& importOptions)
    : Resource(uid, name, ResourceType::Material)
{
    if (importOptions.HasMember("defaultTextureUID") && importOptions["defaultTextureUID"].IsUint64())
        defaultTextureUID = importOptions["defaultTextureUID"].GetUint64();

    else defaultTextureUID = INVALID_UID;
}

ResourceMaterial::~ResourceMaterial()
{
    FreeMaterials();
}

void ResourceMaterial::OnEditorUpdate()
{
    bool updated = false;

    if (diffuseTexture.textureID != 0)
    {
        ImGui::Text("Diffuse Texture");
        ImGui::Image((ImTextureID)(intptr_t)diffuseTexture.textureID, ImVec2(256, 256));
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Texture Dimensions: %d, %d", diffuseTexture.width, diffuseTexture.height);
        }
    }

    updated |= ImGui::SliderFloat3("Diffuse Color", &material.diffColor.x, 0.0f, 1.0f);

    if (metallicTexture.textureID != 0)
    {
        ImGui::Text("Metallic Roughness Texture");
        ImGui::Image((ImTextureID)(intptr_t)metallicTexture.textureID, ImVec2(256, 256));
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Texture Dimensions: %d, %d", metallicTexture.width, metallicTexture.height);
        }
        updated |= ImGui::SliderFloat("Metallic Factor", &material.metallicFactor, 0.0f, 1.0f);
        updated |= ImGui::SliderFloat("Roughness Factor", &material.roughnessFactor, 0.0f, 1.0f);
    }

    else
    {
        if (specularTexture.textureID != 0)
        {
            ImGui::Text("Specular Texture");
            ImGui::Image((ImTextureID)(intptr_t)specularTexture.textureID, ImVec2(256, 256));
            if (ImGui::IsItemHovered())
            {
                ImGui::SetTooltip("Texture Dimensions: %d, %d", specularTexture.width, specularTexture.height);
            }
        }

        updated |= ImGui::SliderFloat3("Specular Color", &material.specColor.x, 0.0f, 1.0f);
        if (!material.shininessInAlpha) updated |= ImGui::SliderFloat("Shininess", &material.shininess, 0.0f, 500.0f);
    }

    if (normalTexture.textureID != 0)
    {
        ImGui::Text("Normal Texture");
        ImGui::Image((ImTextureID)(intptr_t)normalTexture.textureID, ImVec2(256, 256));
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Texture Dimensions: %d, %d", normalTexture.width, normalTexture.height);
        }
    }
}

void ResourceMaterial::LoadMaterialData(Material mat)
{
    material.diffColor           = mat.GetDiffuseFactor();
    material.specColor           = mat.GetSpecularFactor();
    material.shininess           = mat.GetGlossinessFactor();
    material.metallicFactor      = mat.GetMetallicFactor();
    material.roughnessFactor     = mat.GetRoughnessFactor();
    material.shininessInAlpha    = false;

    ResourceTexture* diffTexture = TextureImporter::LoadTexture(mat.GetDiffuseTexture());
    if (diffTexture != nullptr)
    {
        diffuseTexture.textureID = diffTexture->GetTextureID();

        material.diffuseTex      = glGetTextureHandleARB(diffTexture->GetTextureID());
        glMakeTextureHandleResidentARB(material.diffuseTex);

        diffuseTexture.width  = diffTexture->GetTextureWidth();
        diffuseTexture.height = diffTexture->GetTextureHeight();
    }

    if (diffuseTexture.textureID == 0)
    {
        ResourceTexture* diffTexture = TextureImporter::LoadTexture(FALLBACK_TEXTURE_UID);
        if (diffTexture != nullptr)
        {
            diffuseTexture.textureID = diffTexture->GetTextureID();

            material.diffuseTex      = glGetTextureHandleARB(diffTexture->GetTextureID());
            glMakeTextureHandleResidentARB(material.diffuseTex);

            diffuseTexture.width  = diffTexture->GetTextureWidth();
            diffuseTexture.height = diffTexture->GetTextureHeight();
        }
    }

    ResourceTexture* metallicRoughnessTexture = TextureImporter::LoadTexture(mat.GetMetallicRoughnessTexture());
    if (metallicRoughnessTexture != nullptr)
    {
        metallicTexture.textureID = metallicRoughnessTexture->GetTextureID();

        material.metallicTex      = glGetTextureHandleARB(metallicRoughnessTexture->GetTextureID());
        glMakeTextureHandleResidentARB(material.metallicTex);

        metallicTexture.width  = metallicRoughnessTexture->GetTextureWidth();
        metallicTexture.height = metallicRoughnessTexture->GetTextureHeight();
    }

    if (metallicTexture.textureID == 0)
    {
        ResourceTexture* specTexture = TextureImporter::LoadTexture(mat.GetSpecularGlossinessTexture());
        if (specTexture != nullptr)
        {
            specularTexture.textureID = specTexture->GetTextureID();

            material.specularTex      = glGetTextureHandleARB(specTexture->GetTextureID());
            glMakeTextureHandleResidentARB(material.specularTex);

            specularTexture.width     = specTexture->GetTextureWidth();
            specularTexture.height    = specTexture->GetTextureHeight();

            material.shininessInAlpha = true;
        }

        delete specTexture;
    }

    ResourceTexture* normTexture = TextureImporter::LoadTexture(mat.GetNormalTexture());
    if (normTexture != nullptr)
    {
        // GLOG("%s has normal", normTexture->GetName().c_str());
        normalTexture.textureID = normTexture->GetTextureID();

        material.normalTex      = glGetTextureHandleARB(normTexture->GetTextureID());
        glMakeTextureHandleResidentARB(material.normalTex);

        normalTexture.width  = normTexture->GetTextureWidth();
        normalTexture.height = normTexture->GetTextureHeight();
    }

    delete diffTexture;
    delete metallicRoughnessTexture;
    delete normTexture;
}

void ResourceMaterial::FreeMaterials() const
{
    glMakeTextureHandleNonResidentARB(material.diffuseTex);
    glDeleteTextures(1, &diffuseTexture.textureID);

    glMakeTextureHandleNonResidentARB(material.specularTex);
    glDeleteTextures(1, &specularTexture.textureID);

    glMakeTextureHandleNonResidentARB(material.metallicTex);
    glDeleteTextures(1, &metallicTexture.textureID);

    glMakeTextureHandleNonResidentARB(material.normalTex);
    glDeleteTextures(1, &normalTexture.textureID);
}