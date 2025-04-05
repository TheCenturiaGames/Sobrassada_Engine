#include "SphereColliderComponent.h"

#include "Application.h"
#include "GameObject.h"
#include "PhysicsModule.h"

#include "Geometry/Sphere.h"
#include "ImGui.h"
#include "Libs/rapidjson/document.h"

SphereColliderComponent::SphereColliderComponent(UID uid, GameObject* parent)
    : Component(uid, parent, "Sphere Collider", COMPONENT_SPHERE_COLLIDER)
{

    CalculateCollider();

    onCollissionCallback = CollisionDelegate(
        std::bind(&SphereColliderComponent::OnCollision, this, std::placeholders::_1, std::placeholders::_2)
    );

    userPointer = BulletUserPointer(this, &onCollissionCallback, generateCallback);
    App->GetPhysicsModule()->CreateSphereRigidBody(this);
}

SphereColliderComponent::SphereColliderComponent(const rapidjson::Value& initialState, GameObject* parent)
    : Component(initialState, parent)
{
    if (initialState.HasMember("FreezeRotation")) freezeRotation = initialState["FreezeRotation"].GetBool();
    if (initialState.HasMember("Mass")) mass = initialState["Mass"].GetFloat();
    if (initialState.HasMember("ColliderType")) colliderType = ColliderType(initialState["ColliderType"].GetInt());
    if (initialState.HasMember("ColliderLayer")) layer = ColliderLayer(initialState["ColliderLayer"].GetInt());
    if (initialState.HasMember("GenerateCallback")) generateCallback = initialState["GenerateCallback"].GetBool();
    if (initialState.HasMember("Radius")) radius = initialState["Radius"].GetFloat();
    if (initialState.HasMember("FitToSize")) fitToSize = initialState["FitToSize"].GetBool();

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
        std::bind(&SphereColliderComponent::OnCollision, this, std::placeholders::_1, std::placeholders::_2)
    );

    userPointer = BulletUserPointer(this, &onCollissionCallback, generateCallback);
    App->GetPhysicsModule()->CreateSphereRigidBody(this);
}

SphereColliderComponent::~SphereColliderComponent()
{
    App->GetPhysicsModule()->DeleteSphereRigidBody(this);
}

void SphereColliderComponent::Save(rapidjson::Value& targetState, rapidjson::Document::AllocatorType& allocator) const
{
    Component::Save(targetState, allocator);

    targetState.AddMember("FreezeRotation", freezeRotation, allocator);
    targetState.AddMember("Mass", mass, allocator);
    targetState.AddMember("ColliderType", (int)colliderType, allocator);
    targetState.AddMember("ColliderLayer", (int)layer, allocator);
    targetState.AddMember("Radius", radius, allocator);
    targetState.AddMember("GenerateCallback", generateCallback, allocator);
    targetState.AddMember("FitToSize", fitToSize, allocator);

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

void SphereColliderComponent::Clone(const Component* other)
{
}

void SphereColliderComponent::RenderEditorInspector()
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
                    parent->UpdateMobilityHierarchy(MobilitySettings::STATIC);
                    mass = 0.f;
                }

                else if (colliderType == ColliderType::DYNAMIC)
                {
                    parent->UpdateMobilityHierarchy(MobilitySettings::DYNAMIC);
                    mass = 1.f;
                }

                App->GetPhysicsModule()->UpdateSphereRigidBody(this);
            }
        }
        ImGui::EndCombo();
    }

    ImGui::BeginDisabled(colliderType == ColliderType::STATIC);
    if (ImGui::InputFloat("Mass", &mass))
    {
        App->GetPhysicsModule()->UpdateSphereRigidBody(this);
    }
    ImGui::EndDisabled();

    if (ImGui::InputFloat3("Center offset", &centerOffset[0])) App->GetPhysicsModule()->UpdateSphereRigidBody(this);

    if (ImGui::InputFloat("Radius", &radius)) App->GetPhysicsModule()->UpdateSphereRigidBody(this);

    if (ImGui::Checkbox("Freeze rotation", &freezeRotation)) App->GetPhysicsModule()->UpdateSphereRigidBody(this);

    if (ImGui::InputFloat3("Center rotation", &centerRotation[0])) App->GetPhysicsModule()->UpdateSphereRigidBody(this);

    // COLLIDER LAYER SETTINGS
    if (ImGui::BeginCombo("Layer options", ColliderLayerStrings[(int)layer]))
    {
        int colliderStringSize = sizeof(ColliderLayerStrings) / sizeof(char*);
        for (int i = 0; i < colliderStringSize; ++i)
        {
            if (ImGui::Selectable(ColliderLayerStrings[i]))
            {
                layer = ColliderLayer(i);
                App->GetPhysicsModule()->UpdateSphereRigidBody(this);
            }
        }
        ImGui::EndCombo();
    }

    if (ImGui::Checkbox("Fit to size", &fitToSize))
    {
        CalculateCollider();
        App->GetPhysicsModule()->UpdateSphereRigidBody(this);
    }

    if (ImGui::Checkbox("Generate Callbacks", &generateCallback))
    {
        userPointer = BulletUserPointer(this, &onCollissionCallback, generateCallback);
    }
}

void SphereColliderComponent::Update(float deltaTime)
{
}

void SphereColliderComponent::Render(float deltaTime)
{
}

void SphereColliderComponent::ParentUpdated()
{
    if (fitToSize) CalculateCollider();

    App->GetPhysicsModule()->UpdateSphereRigidBody(this);
}

void SphereColliderComponent::OnCollision(GameObject* otherObject, float3 collisionNormal)
{
}

void SphereColliderComponent::CalculateCollider()
{
    AABB heriachyAABB = parent->GetHierarchyAABB();
    
    if (heriachyAABB.IsFinite() && !heriachyAABB.IsDegenerate())
    {
        Sphere sphere = heriachyAABB.MinimalEnclosingSphere();

        radius        = sphere.r;
        centerOffset  = heriachyAABB.CenterPoint() - parent->GetPosition();
    }
}
