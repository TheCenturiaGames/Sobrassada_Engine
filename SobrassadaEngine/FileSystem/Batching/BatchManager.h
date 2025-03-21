#pragma once

#include <vector>

class GeometryBatch;
class MeshComponent;

class BatchManager
{
  public:
    BatchManager();
    ~BatchManager();

    GeometryBatch* RequestBatch(const MeshComponent* mesh);

  private:
    GeometryBatch* CreateNewBatch(const MeshComponent* mesh);

  private:
    std::vector<GeometryBatch*> batches;
};
