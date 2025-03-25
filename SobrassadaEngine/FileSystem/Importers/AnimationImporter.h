#pragma once

#include "Globals.h"

#include "tiny_gltf.h"

namespace tinygltf
{
    class Model;
    class Animation;
    class AnimationSampler;
    struct Accessor;
    struct BufferView;
    struct Buffer;
} // namespace tinygltf
class ResourceAnimation;

enum class  AnimationType : uint8_t
{   TRANSLATION,
    ROTATION
};
namespace AnimationImporter
{
    UID ImportAnimation(
        const tinygltf::Model& model, const tinygltf::Animation& animation, const std::string& name,
        const char* filePath
    );
    ResourceAnimation* LoadAnimation(UID animationUID);

}; 