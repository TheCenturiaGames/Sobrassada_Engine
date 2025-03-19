#pragma once
#include "EngineEditors/Editor/NodeEditor.h"
#include "EngineEditors/EngineEditorBase.h"
#include "Module.h"
#include "ResourceManagement/Resources/Resource.h"

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

    std::string RenderFileDialog(bool& window, const char* windowTitle, bool selectFolder = false);

    GizmoOperation& GetCurrentGizmoOperation() { return currentGizmoOperation; }
    GizmoTransform& GetTransformType() { return transformType; }
    float3& GetSnapValues() { return snapValues; }
    GizmoDragState GetImGuizmoDragState() const { return guizmoDragState; };

    void SetFileDialogCurrentPath(char* newProjectPath) { fileDialogCurrentPath = newProjectPath; }

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
    void WindowConfig(bool& vsync);
    void CameraConfig() const;
    void OpenGLConfig();
    void GameTimerConfig() const;
    void HardwareConfig() const;
    void ShowCaps() const;

    void ImportDialog(bool& import);
    void LoadDialog(bool& load);
    void LoadModelDialog(bool& loadModel);
    void LoadPrefabDialog(bool& loadPrefab);
    void SaveDialog(bool& save);
    void Console(bool& consoleMenu) const;
    void About(bool& aboutMenu);
    std::string FormatWithCommas(unsigned int number) const;

    void OpenEditor(EngineEditorBase* editorToOpen);
    EngineEditorBase* CreateEditor(EditorType type);

    void UpdateGizmoDragState();

  public:
    bool editorControlMenu = true;
    bool hierarchyMenu     = true;
    bool inspectorMenu     = true;
    bool lightConfig       = false;
    bool snapEnabled       = false;

  private:
    int width, height;
    bool consoleMenu        = false;
    bool importMenu         = false;
    bool loadMenu           = false;
    bool saveMenu           = false;
    bool loadModel          = false;
    bool loadPrefab         = false;
    bool aboutMenu          = false;
    bool editorSettingsMenu = false;
    bool closeScene         = false;
    bool closeApplication   = false;

    int maximumPlotData     = 50;
    std::deque<float> framerate;
    std::deque<float> frametime;

    std::string scenesPath;

    GizmoOperation currentGizmoOperation = GizmoOperation::TRANSLATE;
    GizmoTransform transformType         = GizmoTransform::LOCAL;
    GizmoDragState guizmoDragState       = GizmoDragState::IDLE;

    float3 snapValues                    = {1.f, 1.f, 1.f};
    std::unordered_map<UID, EngineEditorBase*> openEditors;

    // load dialog
    std::string inputFileLoad = "";
    std::vector<std::string> filesLoad;
    int selectedLoad = -1;

    // save dialog
    char inputFileSave[32];
    std::vector<std::string> filesSave;

    // load prefab dialog
    UID prefabUID              = INVALID_UID;
    char searchTextPrefab[255] = "";
    int selectedPrefab         = -1;

    // load model dialog
    UID modelUID               = INVALID_UID;
    char searchTextModel[255]  = "";
    int selectedModel          = -1;

    // render file dialog
    std::string fileDialogCurrentPath;
    std::vector<std::string> accPaths;
    bool loadButtons = true;
    std::vector<std::string> filesFileDialog;
    bool doLoadFiles = false;
    std::vector<std::string> filteredFiles;
    bool loadFilteredFiles = false;
    char searchQueryFileDialog[32];
    char lastQueryFileDialog[32] = "default";
    bool showDrives              = false;
    std::string inputFileDialog  = "";
    int selectedFileDialog       = -1;

    bool lockScaleAxis           = false;
    bool bUseRad                 = true;

    // render resource select dialog
    char searchTextResource[255] = "";

    // about
    bool showConfigInfo          = false;

    // frame plots
    float maxYAxis               = 0;

    // window config
    bool fullscreen              = false;
    bool full_desktop            = false;
    bool borderless              = false;
    bool resizable               = false;
    bool vsync                   = false;

    // opengl config
    bool depthTest               = true;
    bool faceCulling             = true;
    int frontFaceMode            = 0;
    float lastTimeOpenGL         = 0.f;
    std::string tpsStr;
};
