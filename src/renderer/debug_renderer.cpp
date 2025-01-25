#include "debug_renderer.h"
#include "navigation_mesh.h"

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

void DebugRenderer::drawNavMesh(const NavigationMesh &navMesh, Color wireColor, Color faceColor)
{
    if (!show_nav_mesh)
        return;

    const auto &triangles = navMesh.getTriangles();
    const auto &vertices = navMesh.getVertices();

    for (const auto &triangle : triangles)
    {
        drawNavMeshTriangle(triangle, vertices, wireColor, faceColor);
    }
}

void DebugRenderer::drawNavMeshTriangle(const NavTriangle &triangle,
                                        const std::vector<glm::vec3> &vertices,
                                        Color wireColor, Color faceColor)
{
    // Get triangle vertices
    Vector3 v1 = {vertices[triangle.vertices[0]].x,
                  vertices[triangle.vertices[0]].y,
                  vertices[triangle.vertices[0]].z};
    Vector3 v2 = {vertices[triangle.vertices[1]].x,
                  vertices[triangle.vertices[1]].y,
                  vertices[triangle.vertices[1]].z};
    Vector3 v3 = {vertices[triangle.vertices[2]].x,
                  vertices[triangle.vertices[2]].y,
                  vertices[triangle.vertices[2]].z};

    // Draw filled triangle
    if (!show_wireframe)
    {
        DrawTriangle3D(v1, v2, v3, faceColor);
    }

    // Draw wireframe
    DrawLine3D(v1, v2, wireColor);
    DrawLine3D(v2, v3, wireColor);
    DrawLine3D(v3, v1, wireColor);
}

void DebugRenderer::drawNavMeshBounds(const NavigationMesh &navMesh)
{
    auto bounds = navMesh.getBounds();

    // Draw wireframe box
    DrawCubeWires(
        {(bounds.min.x + bounds.max.x) / 2,
         (bounds.min.y + bounds.max.y) / 2,
         (bounds.min.z + bounds.max.z) / 2},
        bounds.max.x - bounds.min.x,
        bounds.max.y - bounds.min.y,
        bounds.max.z - bounds.min.z,
        YELLOW);
}