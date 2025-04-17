#pragma once

struct NavMeshSettings {
    float cellSize = 0.3f;
    float cellHeight = 0.2f;
    float walkableSlopeAngle = 45.0f;
    int walkableClimb = 4;
    int walkableHeight = 2;
    int walkableRadius = 2;
    int maxEdgeLen = 12;
    float maxSimplificationError = 1.3f;
    int minRegionArea = 8;
    int mergeRegionArea = 20;
    int maxVertsPerPoly = 6;
    float detailSampleDist = 6.0f;
    float detailSampleMaxError = 1.0f;

    // Your custom params
    bool filterLowHangingObstacles = true;
    bool filterLedgeSpans = true;
    bool filterWalkableLowHeightSpans = true;

    float agentHeight = 2.0f;
    float agentRadius = 0.5f;
    float agentMaxClimb = 0.9f;
};

class NavMeshConfig {
public:
    NavMeshConfig();

    void ApplyTo(void* outRecastRcConfig) const; // opaque interface
    void RenderEditorUI();
    const NavMeshSettings& GetSettings() const { return settings; }

private:
    NavMeshSettings settings;
};