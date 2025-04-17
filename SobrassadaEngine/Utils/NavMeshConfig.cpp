#include "NavMeshConfig.h"
#include "Recast.h" // Only here!
#include "imgui.h"

#include <memory>

void NavMeshConfig::ApplyTo(void* out) const {
    rcConfig& outCfg = *reinterpret_cast<rcConfig*>(out);
    memset(&outCfg, 0, sizeof(rcConfig));

    outCfg.cs = settings.cellSize;
    outCfg.ch = settings.cellHeight;
    outCfg.walkableSlopeAngle = settings.walkableSlopeAngle;
    outCfg.walkableClimb = settings.walkableClimb;
    outCfg.walkableHeight = settings.walkableHeight;
    outCfg.walkableRadius = settings.walkableRadius;
    outCfg.maxEdgeLen = settings.maxEdgeLen;
    outCfg.maxSimplificationError = settings.maxSimplificationError;
    outCfg.minRegionArea = settings.minRegionArea;
    outCfg.mergeRegionArea = settings.mergeRegionArea;
    outCfg.maxVertsPerPoly = settings.maxVertsPerPoly;
    outCfg.detailSampleDist = settings.detailSampleDist;
    outCfg.detailSampleMaxError = settings.detailSampleMaxError;


}

void NavMeshConfig::RenderEditorUI() {
    ImGui::Text("Recast Config");

    ImGui::SliderFloat("Cell Size", &settings.cellSize, 0.05f, 1.0f);
    ImGui::SliderFloat("Cell Height", &settings.cellHeight, 0.05f, 1.0f);

    ImGui::SliderFloat("Slope Angle", &settings.walkableSlopeAngle, 0.0f, 90.0f);
    ImGui::SliderInt("Walkable Climb", &settings.walkableClimb, 0, 10);
    ImGui::SliderInt("Walkable Height", &settings.walkableHeight, 0, 10);
    ImGui::SliderInt("Walkable Radius", &settings.walkableRadius, 0, 10);

    ImGui::SliderInt("Max Edge Length", &settings.maxEdgeLen, 0, 50);
    ImGui::SliderFloat("Max Simplification Error", &settings.maxSimplificationError, 0.0f, 5.0f);
    ImGui::SliderInt("Min Region Area", &settings.minRegionArea, 0, 100);
    ImGui::SliderInt("Merge Region Area", &settings.mergeRegionArea, 0, 100);
    ImGui::SliderInt("Max Verts Per Poly", &settings.maxVertsPerPoly, 3, 12);

    ImGui::SliderFloat("Detail Sample Distance", &settings.detailSampleDist, 0.0f, 10.0f);
    ImGui::SliderFloat("Detail Sample Max Error", &settings.detailSampleMaxError, 0.0f, 10.0f);

    ImGui::Separator();
    ImGui::Text("Additional Filters");

    ImGui::Checkbox("Filter Low Hanging Obstacles", &settings.filterLowHangingObstacles);
    ImGui::Checkbox("Filter Ledge Spans", &settings.filterLedgeSpans);
    ImGui::Checkbox("Filter Walkable Low Height Spans", &settings.filterWalkableLowHeightSpans);

    ImGui::Separator();
    ImGui::Text("Agent Parameters");

    ImGui::SliderFloat("Agent Height", &settings.agentHeight, 0.0f, 5.0f);
    ImGui::SliderFloat("Agent Radius", &settings.agentRadius, 0.0f, 5.0f);
    ImGui::SliderFloat("Agent Max Climb", &settings.agentMaxClimb, 0.0f, 5.0f);
}