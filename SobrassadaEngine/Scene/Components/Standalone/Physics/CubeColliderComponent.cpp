#include "CubeColliderComponent.h"

#include "Application.h"
#include "GameObject.h"
#include "PhysicsModule.h"

#include "ImGui.h"
#include "Libs/rapidjson/document.h"

CubeColliderComponent::CubeColliderComponent(UID uid, GameObject* parent)
    : Component(uid, parent, "Cube Collider", COMPONENT_CUBE_COLLIDER)
{

    AABB heriachyAABB    = parent->GetHeriachyAABB();
    size                 = heriachyAABB.HalfSize();
    centerOffset         = heriachyAABB.CenterPoint() - parent->GetPosition();

    onCollissionCallback = CollisionDelegate(
        std::bind(&CubeColliderComponent::OnCollision, this, std::placeholders::_1, std::placeholders::_2)
    );
    userPointer = BulletUserPointer(this, &onCollissionCallback);
    App->GetPhysicsModule()->CreateCubeRigidBody(this);
}

CubeColliderComponent::CubeColliderComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent)
{
    if (initialState.HasMember("FreezeRotation")) freezeRotation = initialState["FreezeRotation"].GetBool();

    if (initialState.HasMember("Mass")) mass = initialState["Mass"].GetFloat();

    if (initialState.HasMember("ColliderType")) colliderType = ColliderType(initialState["ColliderType"].GetInt());
    if (initialState.HasMember("ColliderLayer")) layer = ColliderLayer(initialState["ColliderLayer"].GetInt());

    if (initialState.HasMember("CenterOffset"))
    {
        const rapidjson::Value& dataArray = initialState["CenterOffset"];
        centerOffset                      = {dataArray[0].GetFloat(), dataArray[1].GetFloat(), dataArray[2].GetFloat()};
    }

    if (initialState.HasMember("CenterRotation"))
    {
        const rapidjson::Value& dataArray = initialState["CenterRotation"];
        centerRotation                    = {dataArray[0].GetFloat(), dataArray[1].GetFloat(), dataArray[2].GetFloat()};
    }

    if (initialState.HasMember("Size"))
    {
        const rapidjson::Value& dataArray = initialState["Size"];
        size                              = {dataArray[0].GetFloat(), dataArray[1].GetFloat(), dataArray[2].GetFloat()};
    }

    onCollissionCallback = CollisionDelegate(
        std::bind(&CubeColliderComponent::OnCollision, this, std::placeholders::_1, std::placeholders::_2)
    );
    userPointer = BulletUserPointer(this, &onCollissionCallback);
    App->GetPhysicsModule()->CreateCubeRigidBody(this);
}

CubeColliderComponent::~CubeColliderComponent()
{
    App->GetPhysicsModule()->DeleteCubeRigidBody(this);
}

void CubeColliderComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);

    targetState.AddMember("FreezeRotation", freezeRotation, allocator);
    targetState.AddMember("Mass", mass, allocator);
    targetState.AddMember("ColliderType", (int)colliderType, allocator);
    targetState.AddMember("ColliderLayer", (int)layer, allocator);

    // CENTER OFFSET
    rapidjson::Value centerOffsetSave(rapidjson::kArrayType);
    centerOffsetSave.PushBack(centerOffset.x, allocator)
        .PushBack(centerOffset.y, allocator)
        .PushBack(centerOffset.z, allocator);
    targetState.AddMember("CenterOffset", centerOffsetSave, allocator);

    // CENTER ROTATION
    rapidjson::Value centerRotationSave(rapidjson::kArrayType);
    centerRotationSave.PushBack(centerRotation.x, allocator)
        .PushBack(centerRotation.y, allocator)
        .PushBack(centerRotation.z, allocator);
    targetState.AddMember("CenterRotation", centerRotationSave, allocator);

    // BOX SIZE
    rapidjson::Value sizeSave(rapidjson::kArrayType);
    sizeSave.PushBack(size.x, allocator).PushBack(size.y, allocator).PushBack(size.z, allocator);
    targetState.AddMember("Size", sizeSave, allocator);
}

void CubeColliderComponent::Clone(const Component* other)
{
}

void CubeColliderComponent::RenderEditorInspector()
{

    if (ImGui::BeginCombo("Collider type", ColliderTypeStrings[(int)colliderType]))
    {
        int colliderStringSize = sizeof(ColliderTypeStrings) / sizeof(char*);
        for (int i = 0; i < colliderStringSize; ++i)
        {
            if (ImGui::Selectable(ColliderTypeStrings[i]))
            {
                colliderType = ColliderType(i);
                if (colliderType == ColliderType::STATIC)
                {
                    mass = 0.f;
                    parent->UpdateMobilityHeriarchy(MobilitySettings::STATIC);
                }
                else if (colliderType == ColliderType::DYNAMIC)
                    parent->UpdateMobilityHeriarchy(MobilitySettings::DYNAMIC);
                App->GetPhysicsModule()->UpdateCubeRigidBody(this);
            }
        }
        ImGui::EndCombo();
    }

    ImGui::BeginDisabled(colliderType != ColliderType::DYNAMIC);
    if (ImGui::InputFloat("Mass", &mass))
    {
        if (mass == 0.f) colliderType = ColliderType::STATIC;
        App->GetPhysicsModule()->UpdateCubeRigidBody(this);
    }
    ImGui::EndDisabled();

    if (ImGui::InputFloat3("Center offset", &centerOffset[0])) App->GetPhysicsModule()->UpdateCubeRigidBody(this);

    if (ImGui::InputFloat3("Size", &size[0])) App->GetPhysicsModule()->UpdateCubeRigidBody(this);

    if (ImGui::Checkbox("Freeze rotation", &freezeRotation)) App->GetPhysicsModule()->UpdateCubeRigidBody(this);

    if (ImGui::InputFloat3("Center rotation", &centerRotation[0])) App->GetPhysicsModule()->UpdateCubeRigidBody(this);

    // COLLIDER LAYER SETTINGS
    if (ImGui::BeginCombo("Layer options", ColliderLayerStrings[(int)layer]))
    {
        int colliderStringSize = sizeof(ColliderLayerStrings) / sizeof(char*);
        for (int i = 0; i < colliderStringSize; ++i)
        {
            if (ImGui::Selectable(ColliderLayerStrings[i]))
            {
                layer = ColliderLayer(i);
                App->GetPhysicsModule()->UpdateCubeRigidBody(this);
            }
        }
        ImGui::EndCombo();
    }
}

void CubeColliderComponent::Update(float deltaTime)
{
}

void CubeColliderComponent::Render(float deltaTime)
{
}

void CubeColliderComponent::ParentUpdated()
{
    AABB heriachyAABB = parent->GetHeriachyAABB();
    size              = heriachyAABB.HalfSize();
    centerOffset      = heriachyAABB.CenterPoint() - parent->GetPosition();

    App->GetPhysicsModule()->UpdateCubeRigidBody(this);
}

void CubeColliderComponent::OnCollision(GameObject* otherObject, float3 collisionNormal)
{
}
