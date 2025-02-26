#include "PrimitiveMesh.h"
#include "Application.h"
#include "ResourcesModule.h"
#include "CameraModule.h"

PrimitiveMesh::PrimitiveMesh(UID uid, UID uidParent, UID uidRoot, const Transform& parentGlobalTransform, const int primitiveType)
    : Component(uid, uidParent, uidRoot, "Primitive Mesh component", COMPONENT_PRIMITIVE_MESH, parentGlobalTransform)
{
    currentMesh = new ResourceMesh(LCG().IntFast(), "Primitive", {0.5f, 0.5f, 0.5f}, {-0.5f, -0.5f, -0.5f});
    currentMesh->CreatePrimitive(static_cast<PrimitiveType>(primitiveType));
    currentMeshName = currentMesh->GetName();

    localComponentAABB = AABB(currentMesh->GetAABB());
    globalComponentAABB   = AABB(currentMesh->GetAABB());
    AABBUpdatable* parent = GetParent();
    if (parent != nullptr)
    {
        parent->PassAABBUpdateToParent();
    }
}

void PrimitiveMesh::Update()
{
    Render();
}

void PrimitiveMesh::Render()
{
    if (enabled && currentMesh != nullptr)
    {
        unsigned int cameraUBO = App->GetCameraModule()->GetUbo();

        float4x4 model         = float4x4::FromTRS(
            globalTransform.position,
            Quat::FromEulerXYZ(globalTransform.rotation.x, globalTransform.rotation.y, globalTransform.rotation.z),
            globalTransform.scale
        );
        currentMesh->Render(App->GetResourcesModule()->GetProgram(), model, cameraUBO, currentMaterial);
    }
    Component::Render();
}
