#pragma once

#include <vector>

class GeometryBatch;
class MeshComponent;

class BatchManager
{
  public:
    BatchManager();
    ~BatchManager();

    void LoadData();
    void Render(const std::vector<MeshComponent*>& meshesToRender);

    GeometryBatch* RequestBatch(const MeshComponent* mesh);

  private:
    GeometryBatch* CreateNewBatch(const MeshComponent* mesh);

  private:
    std::vector<GeometryBatch*> batches;
};
