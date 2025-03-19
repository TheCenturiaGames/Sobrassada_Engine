#include "TextureLibraryEditor.h"
#include "Application.h"
#include "LibraryModule.h"
#include "ResourceManagement/Resources/ResourceTexture.h"
#include "imgui.h"
#include <TextureImporter.h>

TextureLibraryEditor::TextureLibraryEditor(const std::string& editorName, UID uid) : EngineEditorBase(editorName, uid)
{
}

TextureLibraryEditor::~TextureLibraryEditor()
{
}

bool TextureLibraryEditor::RenderEditor()
{
    if (!ImGui::Begin(name.c_str()))
    {
        ImGui::End();
        return true;
    }

    const auto& textureMap = App->GetLibraryModule()->GetTextureMap();

    if (textureMap.empty())
    {
        ImGui::Text("No textures loaded.");
        ImGui::End();
        return true;
    }

    ImGui::Text("Loaded Textures:");
    ImGui::Separator();

    ImGui::Columns(4, "textures", false);
    ImGui::Text("Name");
    ImGui::NextColumn();
    ImGui::Text("Width");
    ImGui::NextColumn();
    ImGui::Text("Height");
    ImGui::NextColumn();
    ImGui::Text("Memory Size (KB)");
    ImGui::NextColumn();
    ImGui::Separator();

    for (const auto& [name, textureUID] : textureMap)
    {
        ResourceTexture* texture = TextureImporter::LoadTexture(textureUID);
        if (!texture) continue;

        int width  = texture->GetTextureWidth();
        int height = texture->GetTextureHeight();
        int sizeKB = (width * height * 4) / 1024; // 4 bytes per pixel (RGBA)

        ImGui::Text("%s", name.c_str());
        ImGui::NextColumn();
        ImGui::Text("%d", width);
        ImGui::NextColumn();
        ImGui::Text("%d", height);
        ImGui::NextColumn();
        ImGui::Text("%d KB", sizeKB);
        ImGui::NextColumn();
    }

    ImGui::Columns(1);
    ImGui::End();

    return true;
}
