#include "BatchManager.h"

#include "GeometryBatch.h"
#include <Application.h>
#include <CameraComponent.h>
#include <CameraModule.h>
#include <ResourceMaterial.h>
#include <ResourceMesh.h>
#include <Scene.h>
#include <SceneModule.h>
#include <ShaderModule.h>
#include <Standalone/MeshComponent.h>

#include <Math/float3.h>

BatchManager::BatchManager()
{
}

BatchManager::~BatchManager()
{
    for (GeometryBatch* it : batches)
    {
        delete it;
    }
    batches.clear();
    batches.shrink_to_fit();
}

void BatchManager::LoadData()
{
    for (GeometryBatch* it : batches)
        it->LoadData();
}

void BatchManager::Render()
{
    unsigned int cameraUBO = App->GetCameraModule()->GetUbo();

    if (App->GetSceneModule()->GetInPlayMode() && App->GetSceneModule()->GetScene()->GetMainCamera() != nullptr)
        cameraUBO = App->GetSceneModule()->GetScene()->GetMainCamera()->GetUbo();

    float3 cameraPos;
    if (App->GetSceneModule()->GetScene()->GetMainCamera() != nullptr && App->GetSceneModule()->GetInPlayMode())
        cameraPos = App->GetSceneModule()->GetScene()->GetMainCamera()->GetCameraPosition();
    else cameraPos = App->GetCameraModule()->GetCameraPosition();

    for (GeometryBatch* it : batches)
    {
        unsigned int program = it->GetIsMetallic() ? App->GetShaderModule()->GetMetallicRoughnessProgram()
                                                   : App->GetShaderModule()->GetSpecularGlossinessProgram();
        it->Render(program, cameraUBO, cameraPos);
    }
}

void BatchManager::ClearObjectsToRender()
{
    for (GeometryBatch* it : batches)
        it->ClearObjectsToRender();
}

GeometryBatch* BatchManager::RequestBatch(const MeshComponent* component)
{
    if (batches.empty())
    {
        return CreateNewBatch(component);
    }

    const ResourceMesh* mesh         = component->GetResourceMesh();
    const ResourceMaterial* material = component->GetResourceMaterial();

    for (GeometryBatch* it : batches)
    {
        if (it->GetMode() == mesh->GetMode() && it->GetIsMetallic() == material->GetIsMetallicRoughness())
        {
            return it;
        }
    }

    return CreateNewBatch(component);
}

GeometryBatch* BatchManager::CreateNewBatch(const MeshComponent* component)
{
    GeometryBatch* newBatch = new GeometryBatch(component);
    batches.push_back(newBatch);
    return newBatch;
}
