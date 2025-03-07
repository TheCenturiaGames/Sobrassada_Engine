#pragma once

#include "Module.h"
#include "ResourceManagement/Resources/Resource.h"

#include "SDL.h"
#include "imgui_internal.h"
#include <Math/float4x4.h>
#include <deque>
#include <string>
#include <unordered_map>
// imguizmo include after imgui
#include "./Libs/ImGuizmo/ImGuizmo.h"

struct CPUFeature
{
    SDL_bool (*check)();
    const char* name;
};

class EditorViewport;
class QuadtreeViewer;

class EditorUIModule : public Module
{
  public:
    EditorUIModule();
    ~EditorUIModule() override;

    bool Init() override;
    update_status PreUpdate(float deltaTime) override;
    update_status Update(float deltaTime) override;
    update_status RenderEditor(float deltaTime) override;
    update_status PostUpdate(float deltaTime) override;
    bool ShutDown() override;

    bool RenderTransformWidget(float4x4& localTransform, float4x4& globalTransform, const float4x4& parentTransform);
    bool RenderImGuizmo(float4x4& localTransform, float4x4& globalTransform, const float4x4& parentTransform) const;
    UID RenderResourceSelectDialog(const char* id, const std::unordered_map<std::string, UID>& availableResources);

  private:
    void RenderBasicTransformModifiers(
        float3& outputPosition, float3& outputRotation, float3& outputScale, bool& lockScaleAxis,
        bool& positionValueChanged, bool& rotationValueChanged, bool& scaleValueChanged
    );

    void UpdateGizmoTransformMode();
    void LimitFPS(float deltaTime) const;
    void AddFramePlotData(float deltaTime);

    void Draw();
    void MainMenu();
    void EditorSettings(bool& editorSettingsMenu);

    void FramePlots(bool& vsync);
    void WindowConfig(bool& vsync) const;
    void CameraConfig() const;
    void OpenGLConfig() const;
    void GameTimerConfig() const;
    void HardwareConfig() const;
    void ShowCaps() const;

    void ImportDialog(bool& importMenu);
    void GetFilesSorted(const std::string& currentPath, std::vector<std::string>& files);
    void LoadDialog(bool& loadMenu);
    void SaveDialog(bool& saveMenu);
    void Console(bool& consoleMenu) const;
    void About(bool& aboutMenu) const;

  public:
    bool editorControlMenu = true;
    bool hierarchyMenu = true;
    bool inspectorMenu = true;

  private:
    int width, height;
    bool consoleMenu            = false;
    bool importMenu             = false;
    bool loadMenu               = false;
    bool saveMenu               = false;
    bool aboutMenu              = false;
    bool editorSettingsMenu     = false;
    bool quadtreeViewerViewport = false;
    bool closeApplication       = false;

    int maxFPS                  = 60;
    int maximumPlotData         = 50;
    std::deque<float> framerate;
    std::deque<float> frametime;

    ImGuizmo::MODE transformType   = ImGuizmo::LOCAL;

    QuadtreeViewer* quadtreeViewer = nullptr;

    std::string startPath;
    std::string libraryPath;

    ImGuizmo::OPERATION mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
    ;
};