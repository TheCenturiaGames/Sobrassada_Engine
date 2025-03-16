#pragma once

#define NOMINMAX
#include "Algorithm/Random/LCG.h"
#include <Geometry/AABB.h>
#include <stdio.h>
#include <vector>
#include <windows.h>

extern std::vector<char*>* Logs;

#define GLOG(format, ...) glog(__FILE__, __LINE__, format, __VA_ARGS__);

void glog(const char file[], int line, const char* format, ...);

enum update_status
{
    UPDATE_CONTINUE = 1,
    UPDATE_STOP,
    UPDATE_ERROR,
    UPDATE_RESTART
};

// Deletes an array of buffers
#define RELEASE_ARRAY(x)                                                                                               \
    {                                                                                                                  \
        if (x != nullptr)                                                                                              \
        {                                                                                                              \
            delete[] x;                                                                                                \
            x = nullptr;                                                                                               \
        }                                                                                                              \
    }

// Configuration -----------
#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 720

#define FULLSCREEN   false
#define FULL_DESKTOP false
#define BORDERLESS   false
#define RESIZABLE    true
#define VSYNC        true

#define TITLE             "Sobrassada Engine"
#define ENGINE_NAME       "Sobrassada"
#define ORGANIZATION_NAME "Hound of Ulster"
#define ENGINE_VERSION    "0.1.0"
#define HFOV              90

#ifdef _WIN32
#define DELIMITER '\\'
#else
#define DELIMITER '/'
#endif

#define FILENAME_SEPARATOR "_"
#define DEFAULT_SCENE_NAME "New Scene"

#define DEFAULT_GL_CLEAR_COLOR_RED   0.5f
#define DEFAULT_GL_CLEAR_COLOR_GREEN 0.5f
#define DEFAULT_GL_CLEAR_COLOR_BLUE  0.5f

#define DEFAULT_CAMERA_MOVEMENT_SCALE_FACTOR 1.f;
#define DEFAULT_CAMERA_MOVEMENT_SPEED        7.5f;
#define DEFAULT_CAMERA_MOUSE_SENSITIVITY     0.1f;
#define DEFAULT_CAMERA_ROTATE_SENSITIVITY    0.006f;
#define DEFAULT_CAMERA_DRAG_SENSITIVITY      0.05f;
#define DEFAULT_CAMERA_WHEEL_SENSITIVITY     2.f;
#define DEFAULT_CAMERA_ZOOM_SENSITIVITY      0.5f;

#define ENGINE_DEFAULT_ASSETS "EngineDefaults/";
#define ASSETS_PATH         "Assets/"
#define SCENES_PATH         "Assets/Scenes/"
#define METADATA_PATH       "Assets/Metadata/"
#define PREFABS_ASSETS_PATH "Assets/Prefabs/"
#define MODELS_ASSETS_PATH "Assets/Models/"

#define LIBRARY_PATH       "Library/"
#define ANIMATIONS_PATH    "Library/Animations/"
#define AUDIO_PATH         "Library/Audio/"
#define MODELS_LIB_PATH    "Library/Models/"
#define MESHES_PATH        "Library/Meshes/"
#define SCENES_PLAY_PATH   "Library/Scenes/"
#define TEXTURES_PATH      "Library/Textures/"
#define MATERIALS_PATH     "Library/Materials/"
#define PREFABS_LIB_PATH "Library/Prefabs/"

#define ASSET_EXTENSION    ".gltf"
#define MESH_EXTENSION     ".sobrassada"
#define TEXTURE_EXTENSION  ".dds"
#define MATERIAL_EXTENSION ".mat"
#define SCENE_EXTENSION    ".scene"
#define PREFAB_EXTENSION   ".prefab"
#define MODEL_EXTENSION    ".model"
#define META_EXTENSION     ".smeta"

#define MAX_COMPONENT_NAME_LENGTH 64;

// SHADER PATHS
constexpr const char* LIGHTS_VERTEX_SHADER_PATH     = "./EngineDefaults/Shader/Vertex/VertexShader.glsl";

constexpr const char* UNLIT_FRAGMENT_SHADER_PATH    = "./EngineDefaults/Shader/Fragment/UnlitFragmentShader.glsl";
constexpr const char* SPECULAR_FRAGMENT_SHADER_PATH = "./EngineDefaults/Shader/Fragment/BRDFPhongFragmentShader.glsl";
constexpr const char* METALLIC_FRAGMENT_SHADER_PATH = "./EngineDefaults/Shader/Fragment/BRDFCookTorranceShader.glsl";

// DEBUG RENDER OPTIONS
constexpr const char* RENDER_LIGTHS                 = "Render Lights";
constexpr const char* RENDER_WIREFRAME              = "Render Wireframe";
constexpr const char* RENDER_AABB                   = "AABB";
constexpr const char* RENDER_OBB                    = "OBB";
constexpr const char* RENDER_OCTREE                 = "Octree";
constexpr const char* RENDER_CAMERA_RAY             = "Camera Ray";

#define UID uint64_t

constexpr UID INVALID_UID                               = 0;
constexpr UID UID_PREFIX_DIVISOR                        = 100000000000000;
constexpr UID FALLBACK_TEXTURE_UID                      = 1200000000000000;

constexpr const char* CONSTANT_MESH_SELECT_DIALOG_ID    = "mesh-select";
constexpr const char* CONSTANT_TEXTURE_SELECT_DIALOG_ID = "texture-select";
constexpr const char* CONSTANT_PREFAB_SELECT_DIALOG_ID  = "prefab-select";
constexpr const char* CONSTANT_MODEL_SELECT_DIALOG_ID   = "model-select";

constexpr uint32_t CONSTANT_NO_MESH_UUID                = 0;
constexpr uint32_t CONSTANT_NO_TEXTURE_UUID             = 0;

constexpr float PI                                      = 3.14159265359f;
constexpr float RAD_DEGREE_CONV                         = 180.f / PI;
constexpr float DEGREE_RAD_CONV                         = PI / 180.f;

constexpr float MINIMUM_TREE_LEAF_SIZE                  = 1.f;

inline UID GenerateUID()
{
    LCG rng;
    UID uid = static_cast<UID>(rng.IntFast()) << 32 | rng.IntFast(); // Combine two 32-bit values
    return uid;
}

static AABB DEFAULT_GAME_OBJECT_AABB = AABB(float3(-1, -1, -1), float3(1, 1, 1));
