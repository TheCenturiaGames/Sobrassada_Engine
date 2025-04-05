#include "ComponentUtils.h"

#include "CameraComponent.h"
#include "Component.h"
#include "Standalone/CharacterControllerComponent.h"
#include "Standalone/Lights/DirectionalLightComponent.h"
#include "Standalone/Lights/PointLightComponent.h"
#include "Standalone/Lights/SpotLightComponent.h"
#include "Standalone/MeshComponent.h"
#include "Standalone/UI/CanvasComponent.h"
#include "Standalone/UI/Transform2DComponent.h"
#include "Standalone/UI/UILabelComponent.h"
#include "ScriptComponent.h"

#include <cstdint>

Component* ComponentUtils::CreateEmptyComponent(const ComponentType type, const UID uid, GameObject* parent)
{
    Component* generatedComponent;
    switch (type)
    {
    case COMPONENT_NONE:
        return nullptr;
    case COMPONENT_MESH:
        generatedComponent = new MeshComponent(uid, parent);
        break;
    case COMPONENT_POINT_LIGHT:
        generatedComponent = new PointLightComponent(uid, parent);
        break;
    case COMPONENT_SPOT_LIGHT:
        generatedComponent = new SpotLightComponent(uid, parent);
        break;
    case COMPONENT_DIRECTIONAL_LIGHT:
        generatedComponent = new DirectionalLightComponent(uid, parent);
        break;
    case COMPONENT_CHARACTER_CONTROLLER:
        generatedComponent = new CharacterControllerComponent(uid, parent);
        break;
    case COMPONENT_TRANSFORM_2D:
        generatedComponent = new Transform2DComponent(uid, parent);
        break;
    case COMPONENT_CANVAS:
        generatedComponent = new CanvasComponent(uid, parent);
        break;
    case COMPONENT_LABEL:
        generatedComponent = new UILabelComponent(uid, parent);
        break;
    case COMPONENT_CAMERA:
        generatedComponent = new CameraComponent(uid, parent);
        break;
    case COMPONENT_SCRIPT:
        generatedComponent = new ScriptComponent(uid, parent);
        break;
    default:
        return nullptr;
    }

    generatedComponent->Init();

    return generatedComponent;
}

Component* ComponentUtils::CreateExistingComponent(const rapidjson::Value& initialState, GameObject* parent)
{
    if (initialState.HasMember("Type"))
    {
        switch (initialState["Type"].GetInt())
        {
        case COMPONENT_NONE:
            return nullptr;
        case COMPONENT_MESH:
            return new MeshComponent(initialState, parent);
        case COMPONENT_POINT_LIGHT:
            return new PointLightComponent(initialState, parent);
        case COMPONENT_SPOT_LIGHT:
            return new SpotLightComponent(initialState, parent);
        case COMPONENT_DIRECTIONAL_LIGHT:
            return new DirectionalLightComponent(initialState, parent);
        case COMPONENT_CHARACTER_CONTROLLER:
            return new CharacterControllerComponent(initialState, parent);
        case COMPONENT_TRANSFORM_2D:
            return new Transform2DComponent(initialState, parent);
        case COMPONENT_CANVAS:
            return new CanvasComponent(initialState, parent);
        case COMPONENT_LABEL:
            return new UILabelComponent(initialState, parent);
        case COMPONENT_CAMERA:
            return new CameraComponent(initialState, parent);
        case COMPONENT_SCRIPT:
            return new ScriptComponent(initialState, parent);
        default:
            return nullptr;
        }
    }
    return nullptr;
}