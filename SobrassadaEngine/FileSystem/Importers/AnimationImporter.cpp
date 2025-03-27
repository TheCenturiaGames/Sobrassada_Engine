#include "AnimationImporter.h"

#include "Application.h"
#include "FileSystem.h"
#include "LibraryModule.h"
#include "MetaAnimation.h"
#include "ProjectModule.h"
#include "ResourceAnimation.h"

#include <Libs/rapidjson/document.h>
#include <memory>
#include <tiny_gltf.h>
#include <vector>

namespace AnimationImporter
{
    UID ImportAnimation(
        const tinygltf::Model& model, const tinygltf::Animation& animation, const char* sourceFilePath,
        const std::string& targetFilePath, UID sourceUID
    )
    {
        std::vector<char> buffer;

        uint32_t channelCount = static_cast<uint32_t>(animation.channels.size());
        buffer.insert(
            buffer.end(), reinterpret_cast<char*>(&channelCount),
            reinterpret_cast<char*>(&channelCount) + sizeof(uint32_t)
        );

        for (const auto& channel : animation.channels)
        {
            const tinygltf::AnimationSampler& sampler = animation.samplers[channel.sampler];

            const tinygltf::Accessor& input           = model.accessors[sampler.input];
            const tinygltf::Accessor& output          = model.accessors[sampler.output];

            const tinygltf::BufferView& inputView     = model.bufferViews[input.bufferView];
            const tinygltf::BufferView& outputView    = model.bufferViews[output.bufferView];

            const tinygltf::Buffer& inputBuffer       = model.buffers[inputView.buffer];
            const tinygltf::Buffer& outputBuffer      = model.buffers[outputView.buffer];

            const float* timeStamps =
                reinterpret_cast<const float*>(&inputBuffer.data[input.byteOffset + inputView.byteOffset]);
            const float* values =
                reinterpret_cast<const float*>(&outputBuffer.data[output.byteOffset + outputView.byteOffset]);

            size_t keyframeCount        = input.count;

            const std::string& nodeName = model.nodes[channel.target_node].name;
            const uint32_t nameSize     = static_cast<uint32_t>(nodeName.size());

            buffer.insert(
                buffer.end(), reinterpret_cast<const char*>(&nameSize),
                reinterpret_cast<const char*>(&nameSize) + sizeof(uint32_t)
            );
            buffer.insert(buffer.end(), nodeName.begin(), nodeName.end());

            AnimationType animType = AnimationType::TRANSLATION;

            if (channel.target_path == "rotation")
            {
                animType = AnimationType::ROTATION;
            }

            buffer.insert(
                buffer.end(), reinterpret_cast<const char*>(&animType),
                reinterpret_cast<const char*>(&animType) + sizeof(AnimationType)
            );

            buffer.insert(
                buffer.end(), reinterpret_cast<const char*>(&keyframeCount),
                reinterpret_cast<const char*>(&keyframeCount) + sizeof(uint32_t)
            );

            buffer.insert(
                buffer.end(), reinterpret_cast<const char*>(timeStamps),
                reinterpret_cast<const char*>(timeStamps) + keyframeCount * sizeof(float)
            );

            if (animType == AnimationType::TRANSLATION)
            {
                buffer.insert(
                    buffer.end(), reinterpret_cast<const char*>(values),
                    reinterpret_cast<const char*>(values) + keyframeCount * sizeof(float3)
                );
            }
            else
            {
                buffer.insert(
                    buffer.end(), reinterpret_cast<const char*>(values),
                    reinterpret_cast<const char*>(values) + keyframeCount * sizeof(Quat)
                );
            }
        }

        // Handle UID
        UID finalAnimUID;
        if (sourceUID == INVALID_UID)
        {
            UID animationUID      = GenerateUID();
            finalAnimUID          = App->GetLibraryModule()->AssignFiletypeUID(animationUID, FileType::Animation);

            std::string assetPath = ANIMATIONS_PATH + FileSystem::GetFileNameWithExtension(sourceFilePath);
            MetaAnimation meta(finalAnimUID, assetPath);

            
            std::string animName = FileSystem::GetFileNameWithoutExtension(sourceFilePath);
            meta.Save(animName, assetPath);
        }
        else
        {
            finalAnimUID = sourceUID;
        }

        // Construct save paths 
        std::string saveFilePath =
            targetFilePath + ANIMATIONS_PATH + std::to_string(finalAnimUID) + ANIMATION_EXTENSION;

      
        unsigned int bytesWritten =
            (unsigned int)FileSystem::Save(saveFilePath.c_str(), buffer.data(), buffer.size(), true);

        if (bytesWritten == 0)
        {
            GLOG("Failed to save animation file: %s", saveFilePath.c_str());
            return 0;
        }

      
        App->GetLibraryModule()->AddAnimation(finalAnimUID, FileSystem::GetFileNameWithoutExtension(sourceFilePath));
        App->GetLibraryModule()->AddName(FileSystem::GetFileNameWithoutExtension(sourceFilePath), finalAnimUID);
        App->GetLibraryModule()->AddResource(saveFilePath, finalAnimUID);

        GLOG("%s saved as binary", FileSystem::GetFileNameWithoutExtension(sourceFilePath).c_str());

        return finalAnimUID;
    }

    ResourceAnimation* LoadAnimation(UID animationUID)
    {
        rapidjson::Document doc;
        rapidjson::Value importOptions;
        App->GetLibraryModule()->GetImportOptions(animationUID, doc, importOptions);

        const std::string path = App->GetLibraryModule()->GetResourcePath(animationUID);
        const std::string name = App->GetLibraryModule()->GetResourceName(animationUID);

        char* buffer           = nullptr;
        unsigned int fileSize  = FileSystem::Load(path.c_str(), &buffer);

        if (fileSize == 0 || buffer == nullptr)
        {
            GLOG("Failed to load animation file: %s", path.c_str());
            return nullptr;
        }

        char* cursor = buffer;

        uint32_t channelCount;
        memcpy(&channelCount, cursor, sizeof(uint32_t));
        cursor                       += sizeof(uint32_t);

        // Create animation resource with UID and name
        ResourceAnimation* animation  = new ResourceAnimation(animationUID, name);

        // Parse channels
        for (uint32_t i = 0; i < channelCount; ++i)
        {
            uint32_t nameSize;
            memcpy(&nameSize, cursor, sizeof(uint32_t));
            cursor += sizeof(uint32_t);

            std::string nodeName(cursor, nameSize);
            cursor += nameSize;

            AnimationType animType;
            memcpy(&animType, cursor, sizeof(AnimationType));
            cursor += sizeof(AnimationType);

            uint32_t keyframeCount;
            memcpy(&keyframeCount, cursor, sizeof(uint32_t));
            cursor               += sizeof(uint32_t);

            // Get channel reference
            Channel& animChannel  = animation->channels[nodeName];

            // Parse based on animation type
            if (animType == AnimationType::TRANSLATION)
            {
                animChannel.posTimeStamps.resize(keyframeCount);
                memcpy(animChannel.posTimeStamps.data(), cursor, keyframeCount * sizeof(float));
                cursor += keyframeCount * sizeof(float);

                animChannel.positions.resize(keyframeCount);
                memcpy(animChannel.positions.data(), cursor, keyframeCount * sizeof(float3));
                cursor                   += keyframeCount * sizeof(float3);

                animChannel.numPositions  = keyframeCount;
            }
            else
            {
                animChannel.rotTimeStamps.resize(keyframeCount);
                memcpy(animChannel.rotTimeStamps.data(), cursor, keyframeCount * sizeof(float));
                cursor += keyframeCount * sizeof(float);

                animChannel.rotations.resize(keyframeCount);
                memcpy(animChannel.rotations.data(), cursor, keyframeCount * sizeof(Quat));
                cursor                   += keyframeCount * sizeof(Quat);

                animChannel.numRotations  = keyframeCount;
            }
        }

        delete[] buffer;

        animation->SetDuration();

        GLOG("Animation duration: %f", animation->GetDuration());

        return animation;
    }
} // namespace AnimationImporter