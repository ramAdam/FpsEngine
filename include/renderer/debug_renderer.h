
#pragma once
#include "raylib.h"
#include "bsp.h"

class DebugRenderer
{
private:
    bool show_grid{true};
    bool show_wireframe{false};
    bool show_axes{true};
    float grid_spacing{1.0f};
    int grid_slices{10};

public:
    void drawGrid();
    void drawAxes();
    void drawPolygon(const Polygon &poly, Color color);
    void drawPolygons(const std::vector<Polygon> &polys, Color color);
    void toggleGrid() { show_grid = !show_grid; }
    void toggleWireframe() { show_wireframe = !show_wireframe; }
    void setGridSpacing(float spacing) { grid_spacing = spacing; }
};