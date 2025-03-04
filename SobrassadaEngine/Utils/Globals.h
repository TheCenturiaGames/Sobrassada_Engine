#pragma once
#define NOMINMAX
#include <stdio.h>
#include <vector>
#include <windows.h>
#include "Algorithm/Random/LCG.h"

#include <Math/Quat.h>
#include <Math/float4x4.h>

extern std::vector<char *> *Logs;

#define GLOG(format, ...) glog(__FILE__, __LINE__, format, __VA_ARGS__);

void glog(const char file[], int line, const char *format, ...);

enum update_status
{
    UPDATE_CONTINUE = 1,
    UPDATE_STOP,
    UPDATE_ERROR
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

#define BORDERLESS   false
#define FULL_DESKTOP false
#define RESIZABLE    true
#define FULLSCREEN    false
#define VSYNC         true

#define TITLE         "Sobrassada Engine"
#define ENGINE_NAME       "Sobrassada"
#define ORGANIZATION_NAME "Hound of Ulster"
#define ENGINE_VERSION    "0.1.0"
#define HFOV          90

#ifdef _WIN32
#define DELIMITER '\\'
#else
#define DELIMITER '/'
#endif

#define DEFAULT_GL_CLEAR_COLOR_RED   0.5f
#define DEFAULT_GL_CLEAR_COLOR_GREEN 0.5f
#define DEFAULT_GL_CLEAR_COLOR_BLUE  0.5f

#define DEFAULT_CAMERA_MOVEMENT_SCALE_FACTOR 1.f;
#define DEFAULT_CAMERA_MOVEMENT_SPEED        7.5f;
#define DEFAULT_CAMERA_MOUSE_SENSITIVITY     0.5f;
#define DEFAULT_CAMERA_ZOOM_SENSITIVITY      5.f;

#define ASSETS_PATH     "Assets/"
#define LIBRARY_PATH    "Library/"
#define ANIMATIONS_PATH "Library/Animations/"
#define AUDIO_PATH      "Library/Audio/"
#define BONES_PATH      "Library/Bones/"
#define MESHES_PATH     "Library/Meshes/"
#define TEXTURES_PATH   "Library/Textures/"
#define MATERIALS_PATH  "Library/Materials/"
#define SCENES_PATH     "Library/Scenes/"

#define ASSET_EXTENSION    ".gltf"
#define MESH_EXTENSION     ".sobrassada"
#define TEXTURE_EXTENSION  ".dds"
#define MATERIAL_EXTENSION ".mat"
#define SCENE_EXTENSION    ".scene"

#define MAX_COMPONENT_NAME_LENGTH 64;

#define UID uint64_t

constexpr UID INVALID_UUID = 0;
constexpr UID CONSTANT_EMPTY_UID = 0;

constexpr const char* CONSTANT_MESH_SELECT_DIALOG_ID = "mesh-select";
constexpr const char* CONSTANT_TEXTURE_SELECT_DIALOG_ID = "texture-select";

constexpr uint32_t CONSTANT_NO_MESH_UUID = 0;
constexpr uint32_t CONSTANT_NO_TEXTURE_UUID = 0;

constexpr float PI = 3.14159265359f;
constexpr float RAD_DEGREE_CONV = 180.f / PI;

inline UID GenerateUID()
{
    LCG rng;
    
    UID uid = static_cast<UID>(rng.IntFast()) << 32 | rng.IntFast(); // Combine two 32-bit values
    
    return uid;
}
