
#include "debug_renderer.h"

void DebugRenderer::drawGrid()
{
    if (show_grid)
    {
        DrawGrid(grid_slices, grid_spacing);
    }
}

void DebugRenderer::drawAxes()
{
    if (show_axes)
    {
        DrawLine3D({0, 0, 0}, {5, 0, 0}, RED);   // X axis
        DrawLine3D({0, 0, 0}, {0, 5, 0}, GREEN); // Y axis
        DrawLine3D({0, 0, 0}, {0, 0, 5}, BLUE);  // Z axis
    }
}

void DebugRenderer::drawPolygon(const Polygon &poly, Color color)
{
    for (size_t i = 0; i < poly.vertices.size(); ++i)
    {
        const auto &v1 = poly.vertices[i];
        const auto &v2 = poly.vertices[(i + 1) % poly.vertices.size()];
        DrawLine3D(v1, v2, color);
    }
}

void DebugRenderer::drawPolygons(const std::vector<Polygon> &polys, Color color)
{
    for (const auto &poly : polys)
    {
        drawPolygon(poly, color);
    }
}