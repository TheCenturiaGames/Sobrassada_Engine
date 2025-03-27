#pragma once

#include "Globals.h"



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
        const tinygltf::Model& model, const tinygltf::Animation& animation,
        const char* sourceFilePath, const std::string& targetFilePath, UID sourceUID
    );
    ResourceAnimation* LoadAnimation(UID animationUID);
    void CopyAnimation(
        const std::string& filePath, const std::string& targetFilePath, const std::string& name, const UID sourceUID
    );
}; 