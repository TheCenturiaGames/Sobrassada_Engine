#include "CapsuleColliderComponent.h"

#include "Application.h"
#include "GameObject.h"
#include "PhysicsModule.h"

#include "ImGui.h"
#include "Libs/rapidjson/document.h"

CapsuleColliderComponent::CapsuleColliderComponent(UID uid, GameObject* parent)
    : Component(uid, parent, "Capsule Collider", COMPONENT_CAPSULE_COLLIDER)
{

    AABB heriachyAABB    = parent->GetHierarchyAABB();
    radius               = heriachyAABB.Size().MaxElement() / 2.f;
    length               = heriachyAABB.Size().y / 2.f;
    centerOffset         = heriachyAABB.CenterPoint() - parent->GetPosition();

    onCollissionCallback = CollisionDelegate(
        std::bind(&CapsuleColliderComponent::OnCollision, this, std::placeholders::_1, std::placeholders::_2)
    );

    userPointer = BulletUserPointer(this, &onCollissionCallback, generateCallback);
    App->GetPhysicsModule()->CreateCapsuleRigidBody(this);
}

CapsuleColliderComponent::CapsuleColliderComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(uid, parent, "Capsule Collider", COMPONENT_CAPSULE_COLLIDER)
{
    if (initialState.HasMember("FreezeRotation")) freezeRotation = initialState["FreezeRotation"].GetBool();
    if (initialState.HasMember("Mass")) mass = initialState["Mass"].GetFloat();
    if (initialState.HasMember("ColliderType")) colliderType = ColliderType(initialState["ColliderType"].GetInt());
    if (initialState.HasMember("ColliderLayer")) layer = ColliderLayer(initialState["ColliderLayer"].GetInt());
    if (initialState.HasMember("GenerateCallback")) generateCallback = initialState["GenerateCallback"].GetBool();
    if (initialState.HasMember("Radius")) radius = initialState["Radius"].GetFloat();
    if (initialState.HasMember("Length")) length = initialState["Length"].GetFloat();

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

    onCollissionCallback = CollisionDelegate(
        std::bind(&CapsuleColliderComponent::OnCollision, this, std::placeholders::_1, std::placeholders::_2)
    );

    userPointer = BulletUserPointer(this, &onCollissionCallback, generateCallback);
    App->GetPhysicsModule()->CreateCapsuleRigidBody(this);
}

CapsuleColliderComponent::~CapsuleColliderComponent()
{
    App->GetPhysicsModule()->DeleteCapsuleRigidBody(this);
}

void CapsuleColliderComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);

    targetState.AddMember("FreezeRotation", freezeRotation, allocator);
    targetState.AddMember("Mass", mass, allocator);
    targetState.AddMember("ColliderType", (int)colliderType, allocator);
    targetState.AddMember("ColliderLayer", (int)layer, allocator);
    targetState.AddMember("Radius", radius, allocator);
    targetState.AddMember("Length", length, allocator);
    targetState.AddMember("GenerateCallback", generateCallback, allocator);

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
}

void CapsuleColliderComponent::Clone(const Component* other)
{
}

void CapsuleColliderComponent::RenderEditorInspector()
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
                    parent->UpdateMobilityHierarchy(MobilitySettings::STATIC);
                }
                else if (colliderType == ColliderType::DYNAMIC)
                    parent->UpdateMobilityHierarchy(MobilitySettings::DYNAMIC);
                App->GetPhysicsModule()->UpdateCapsuleRigidBody(this);
            }
        }
        ImGui::EndCombo();
    }

    ImGui::BeginDisabled(colliderType != ColliderType::DYNAMIC);
    if (ImGui::InputFloat("Mass", &mass))
    {
        if (mass == 0.f) colliderType = ColliderType::STATIC;
        App->GetPhysicsModule()->UpdateCapsuleRigidBody(this);
    }
    ImGui::EndDisabled();

    if (ImGui::InputFloat3("Center offset", &centerOffset[0])) App->GetPhysicsModule()->UpdateCapsuleRigidBody(this);

    if (ImGui::InputFloat("Radius", &radius)) App->GetPhysicsModule()->UpdateCapsuleRigidBody(this);
    if (ImGui::InputFloat("Length", &length)) App->GetPhysicsModule()->UpdateCapsuleRigidBody(this);

    if (ImGui::Checkbox("Freeze rotation", &freezeRotation)) App->GetPhysicsModule()->UpdateCapsuleRigidBody(this);

    if (ImGui::InputFloat3("Center rotation", &centerRotation[0]))
        App->GetPhysicsModule()->UpdateCapsuleRigidBody(this);

    // COLLIDER LAYER SETTINGS
    if (ImGui::BeginCombo("Layer options", ColliderLayerStrings[(int)layer]))
    {
        int colliderStringSize = sizeof(ColliderLayerStrings) / sizeof(char*);
        for (int i = 0; i < colliderStringSize; ++i)
        {
            if (ImGui::Selectable(ColliderLayerStrings[i]))
            {
                layer = ColliderLayer(i);
                App->GetPhysicsModule()->UpdateCapsuleRigidBody(this);
            }
        }
        ImGui::EndCombo();
    }

    if (ImGui::Checkbox("Generate Callbacks", &generateCallback))
    {
        userPointer = BulletUserPointer(this, &onCollissionCallback, generateCallback);
    }
}

void CapsuleColliderComponent::Update(float deltaTime)
{
}

void CapsuleColliderComponent::Render(float deltaTime)
{
}

void CapsuleColliderComponent::ParentUpdated()
{
    AABB heriachyAABB = parent->GetHierarchyAABB();
    radius            = heriachyAABB.Size().MaxElement() / 2.f;
    length            = heriachyAABB.Size().y / 2.f;
    centerOffset      = heriachyAABB.CenterPoint() - parent->GetPosition();

    App->GetPhysicsModule()->UpdateCapsuleRigidBody(this);
}

void CapsuleColliderComponent::OnCollision(GameObject* otherObject, float3 collisionNormal)
{
}
