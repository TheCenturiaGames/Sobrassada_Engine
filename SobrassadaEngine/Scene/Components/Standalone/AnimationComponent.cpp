#include "AnimationComponent.h"

#include "../Root/RootComponent.h"
#include "Application.h"
#include "CameraModule.h"
#include "EditorUIModule.h"
#include "LibraryModule.h"
#include "ResourcesModule.h"
#include "SceneModule.h"

#include "imgui.h"
#include <Math/Quat.h>

AnimationComponent::AnimationComponent(
    const UID uid, const UID uidParent, const UID uidRoot, const float4x4& parentGlobalTransform
)
    : Component(uid, uidParent, uidRoot, "Mesh", COMPONENT_MESH, parentGlobalTransform)
{
}

AnimationComponent::AnimationComponent(const rapidjson::Value& initialState) : Component(initialState)
{

}
