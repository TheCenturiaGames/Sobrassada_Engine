#pragma once
#include "EngineEditors/EngineEditorBase.h"
#include "Module.h"
#include "ResourceManagement/Resources/Resource.h"
#include "EngineEditors/EngineEditorBase.h"
#include "EngineEditors/Editor/NodeEditor.h"

#include "SDL.h"
#include "imgui_internal.h"
#include <Math/float3.h>
#include <Math/float4x4.h>
#include <deque>
#include <string>
#include <unordered_map>
// imguizmo include after imgui
#include "./Libs/ImGuizmo/ImGuizmo.h"

enum EditorType
{
    BASE,
    ANIMATION,
    NODE
};
struct CPUFeature
{
    SDL_bool (*check)();
    const char* name;
};

enum class GizmoOperation
{
    TRANSLATE,
    ROTATE,
    SCALE
};

enum class GizmoTransform
{
    LOCAL,
    WORLD
};

enum class GizmoDragState
{
    IDLE,
    DRAGGING,
    RELEASED
};

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

    template <typename T>
    T RenderResourceSelectDialog(
        const char* id, const std::unordered_map<std::string, T>& availableResources, const T& defaultResource
    );

    std::string RenderFileDialog(bool& window, const char* windowTitle, bool selectFolder = false) const;

    GizmoOperation& GetCurrentGizmoOperation() { return currentGizmoOperation; }
    GizmoTransform& GetTransformType() { return transformType; }
    float3& GetSnapValues() { return snapValues; }
    GizmoDragState GetImGuizmoDragState() const { return guizmoDragState; };

  private:
    void RenderBasicTransformModifiers(
        float3& outputPosition, float3& outputRotation, float3& outputScale, bool& lockScaleAxis,
        bool& positionValueChanged, bool& rotationValueChanged, bool& scaleValueChanged
    );

    void UpdateGizmoTransformMode();
    ImGuizmo::OPERATION GetImGuizmoOperation() const;
    ImGuizmo::MODE GetImGuizmoTransformMode() const;

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

    void ImportDialog(bool& import);
    void LoadDialog(bool& load);
    void SaveDialog(bool& save);
    void Console(bool& consoleMenu) const;
    void About(bool& aboutMenu) const;
    void NavMesh(bool& navmesh);
    std::string FormatWithCommas(unsigned int number) const;

    void LoadModelDialog(bool& loadModel);

    void OpenEditor(EngineEditorBase* editorToOpen);
    EngineEditorBase* CreateEditor(EditorType type);

    void UpdateGizmoDragState();

  public:
    bool editorControlMenu = true;
    bool hierarchyMenu     = true;
    bool inspectorMenu     = true;
    bool snapEnabled       = false;

  private:
    int width, height;
    bool consoleMenu        = false;
    bool importMenu         = false;
    bool loadMenu           = false;
    bool saveMenu           = false;
    bool loadModel          = false;
    bool aboutMenu          = false;
    bool editorSettingsMenu = false;
    bool closeScene         = false;
    bool closeApplication   = false;
    bool navmesh            = false;


    int maximumPlotData     = 50;
    std::deque<float> framerate;
    std::deque<float> frametime;

    std::string scenesPath;

    GizmoOperation currentGizmoOperation = GizmoOperation::TRANSLATE;
    GizmoTransform transformType         = GizmoTransform::LOCAL;
    GizmoDragState guizmoDragState = GizmoDragState::IDLE;

    float3 snapValues                    = {1.f, 1.f, 1.f};
    std::unordered_map<UID, EngineEditorBase*> openEditors;
};
