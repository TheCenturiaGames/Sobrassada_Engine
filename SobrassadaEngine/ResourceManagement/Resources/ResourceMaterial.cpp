#include "ResourceMaterial.h"

#include "Application.h"
#include "EditorUIModule.h"
#include "FileSystem/Material.h"
#include "LibraryModule.h"
#include "ResourceManagement/Resources/ResourceTexture.h"
#include "TextureImporter.h"

#include "imgui.h"
#include <glew.h>
#include <unordered_set>

ResourceMaterial::ResourceMaterial(UID uid, const std::string& name) : Resource(uid, name, ResourceType::Material)
{
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

        ImGui::SameLine();
        if (ImGui::Button("Select Texture"))
        {
            ImGui::OpenPopup(CONSTANT_TEXTURE_SELECT_DIALOG_ID);
        }

        if (ImGui::IsPopupOpen(CONSTANT_TEXTURE_SELECT_DIALOG_ID))
        {
            UID handle = ChangeTexture(
                App->GetEditorUIModule()->RenderResourceSelectDialog<UID>(
                    CONSTANT_TEXTURE_SELECT_DIALOG_ID, App->GetLibraryModule()->GetTextureMap(), INVALID_UID
                ),
                diffuseTexture, material.diffuseTex
            );

            if (handle != NULL)
            {
                material.diffuseTex = handle;
                updated             = true;
            }
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
        ImGui::SameLine();
        if (ImGui::Button("Select Texture"))
        {
            ImGui::OpenPopup(CONSTANT_TEXTURE_SELECT_DIALOG_ID);
        }

        if (ImGui::IsPopupOpen(CONSTANT_TEXTURE_SELECT_DIALOG_ID))
        {
            UID handle = ChangeTexture(
                App->GetEditorUIModule()->RenderResourceSelectDialog<UID>(
                    CONSTANT_TEXTURE_SELECT_DIALOG_ID, App->GetLibraryModule()->GetTextureMap(), INVALID_UID
                ),
                metallicTexture, material.metallicTex
            );

            if (handle != NULL)
            {
                material.metallicTex = handle;
                updated              = true;
            }
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
        ImGui::SameLine();
        if (ImGui::Button("Select Texture"))
        {
            ImGui::OpenPopup(CONSTANT_TEXTURE_SELECT_DIALOG_ID);
        }

        if (ImGui::IsPopupOpen(CONSTANT_TEXTURE_SELECT_DIALOG_ID))
        {
            UID handle = ChangeTexture(
                App->GetEditorUIModule()->RenderResourceSelectDialog<UID>(
                    CONSTANT_TEXTURE_SELECT_DIALOG_ID, App->GetLibraryModule()->GetTextureMap(), INVALID_UID
                ),
                specularTexture, material.specularTex
            );

            if (handle != NULL)
            {
                material.specularTex = handle;
                updated              = true;
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

        ImGui::SameLine();
        if (ImGui::Button("Select Texture"))
        {
            ImGui::OpenPopup(CONSTANT_TEXTURE_SELECT_DIALOG_ID);
        }

        if (ImGui::IsPopupOpen(CONSTANT_TEXTURE_SELECT_DIALOG_ID))
        {
            UID handle = ChangeTexture(
                App->GetEditorUIModule()->RenderResourceSelectDialog<UID>(
                    CONSTANT_TEXTURE_SELECT_DIALOG_ID, App->GetLibraryModule()->GetTextureMap(), INVALID_UID
                ),
                normalTexture, material.normalTex
            );

            if (handle != NULL)
            {
                material.normalTex = handle;
                updated              = true;
            }
        }
    }

    if (updated) UpdateUBO();
}

UID ResourceMaterial::ChangeTexture(UID newTexture, TextureInfo& textureToChange, UID textureGPU)
{
    if (newTexture == INVALID_UID) return NULL;

    ResourceTexture* texture = TextureImporter::LoadTexture(newTexture);
    if (texture != nullptr)
    {
        glMakeTextureHandleNonResidentARB(textureGPU);
        glDeleteTextures(1, &textureToChange.textureID);

        UID handle = glGetTextureHandleARB(texture->GetTextureID());
        glMakeTextureHandleResidentARB(handle);

        textureToChange.textureID = texture->GetTextureID();
        textureToChange.width     = texture->GetTextureWidth();
        textureToChange.height    = texture->GetTextureHeight();

        return handle;
    }
    delete texture;
    return NULL;
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
        GLOG("%s has normal", normTexture->GetName().c_str());
        normalTexture.textureID = normTexture->GetTextureID();

        material.normalTex      = glGetTextureHandleARB(normTexture->GetTextureID());
        glMakeTextureHandleResidentARB(material.normalTex);

        normalTexture.width  = normTexture->GetTextureWidth();
        normalTexture.height = normTexture->GetTextureHeight();
    }

    glGenBuffers(1, &ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(MaterialGPU), &material, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    delete diffTexture;
    delete metallicRoughnessTexture;
    delete normTexture;
}

void ResourceMaterial::RenderMaterial(int program) const
{
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);

    unsigned int blockIdx = glGetUniformBlockIndex(program, "Material");
    glUniformBlockBinding(program, blockIdx, 1);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, ubo);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
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

    glDeleteBuffers(1, &ubo);
}

void ResourceMaterial::UpdateUBO() const
{
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(MaterialGPU), &material);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}