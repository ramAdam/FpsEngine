#pragma once
#include "raylib.h"
#include "bsp.h"
#include "glm/glm.hpp"

#include <bullet/btBulletDynamicsCommon.h>

class NavigationMesh;
struct NavTriangle; // Add this forward declaration

class DebugRenderer
{
private:
    bool show_grid{true};
    bool show_wireframe{false};
    bool show_axes{true};
    bool show_nav_mesh{false};
    bool showCollisionShapes = false;
    float grid_spacing{1.0f};
    int grid_slices{10};

public:
    void drawGrid();
    void drawAxes();
    void drawPolygon(const Polygon &poly, Color color);
    void drawPolygons(const std::vector<Polygon> &polys, Color color);
    void drawNavMesh(const NavigationMesh &navMesh, Color wireColor = GREEN, Color faceColor = {0, 255, 0, 100});
    void drawNavMeshTriangle(const NavTriangle &triangle,
                             const std::vector<glm::vec3> &vertices,
                             Color wireColor,
                             Color faceColor);
    void drawNavMeshBounds(const NavigationMesh &navMesh);
    void toggleGrid() { show_grid = !show_grid; }
    void toggleWireframe() { show_wireframe = !show_wireframe; }
    void toggleNavMesh() { show_nav_mesh = !show_nav_mesh; }
    void toggleCollisionShapes() { showCollisionShapes = !showCollisionShapes; }
    bool isNavMeshVisible() const { return show_nav_mesh; }
    bool isCollisionDebugVisible() const { return showCollisionShapes; }
    void setGridSpacing(float spacing) { grid_spacing = spacing; }
    void drawCollisionShapes(btDynamicsWorld* dynamicsWorld);
};