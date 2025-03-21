#pragma once

#include <vector>

class GeometryBatch;
class ResourceMesh;

class BatchManager
{
  public:
    BatchManager();
    ~BatchManager();

    GeometryBatch* RequestBatch(const ResourceMesh* mesh);

  private:
    GeometryBatch* CreateNewBatch(const ResourceMesh* mesh);

  private:
    std::vector<GeometryBatch*> batches;
};
