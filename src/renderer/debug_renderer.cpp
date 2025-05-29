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

void DebugRenderer::drawCollisionShapes(btDynamicsWorld* dynamicsWorld) {
    if (!showCollisionShapes) return;
    
    // Debug draw all collision objects
    for (int i = 0; i < dynamicsWorld->getNumCollisionObjects(); i++) {
        btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
        btRigidBody* body = btRigidBody::upcast(obj);
        
        // Get transform
        btTransform trans;
        if (body && body->getMotionState()) {
            body->getMotionState()->getWorldTransform(trans);
        } else {
            trans = obj->getWorldTransform();
        }
        
        // Draw shape based on type
        btCollisionShape* shape = obj->getCollisionShape();
        if (!shape) continue;
        
        // Set color based on object type
        Color color;
        if (obj->getCollisionFlags() & btCollisionObject::CF_STATIC_OBJECT) {
            color = GREEN;  // Static objects in green
        } else if (obj->getCollisionFlags() & btCollisionObject::CF_KINEMATIC_OBJECT) {
            color = BLUE;   // Kinematic objects in blue
        } else {
            color = RED;    // Dynamic objects in red
        }
        
        // Get object position
        btVector3 pos = trans.getOrigin();
        Vector3 position = {pos.x(), pos.y(), pos.z()};
        
        // Draw different shapes based on type
        switch (shape->getShapeType()) {
            case BOX_SHAPE_PROXYTYPE: {
                btBoxShape* box = static_cast<btBoxShape*>(shape);
                btVector3 halfExtents = box->getHalfExtentsWithoutMargin();
                DrawCubeWires(position, halfExtents.x()*2, halfExtents.y()*2, 
                             halfExtents.z()*2, color);
                break;
            }
            case SPHERE_SHAPE_PROXYTYPE: {
                btSphereShape* sphere = static_cast<btSphereShape*>(shape);
                DrawSphereWires(position, sphere->getRadius(), 8, 8, color);
                break;
            }
            case CAPSULE_SHAPE_PROXYTYPE: {
                btCapsuleShape* capsule = static_cast<btCapsuleShape*>(shape);
                float radius = capsule->getRadius();
                float halfHeight = capsule->getHalfHeight();
                
                // Draw cylinder for middle section
                DrawCylinderWires(position, radius, radius, halfHeight*2, 8, color);
                
                // Draw spheres for caps
                Vector3 topPos = {position.x, position.y + halfHeight, position.z};
                Vector3 bottomPos = {position.x, position.y - halfHeight, position.z};
                DrawSphereWires(topPos, radius, 8, 8, color);
                DrawSphereWires(bottomPos, radius, 8, 8, color);
                break;
            }
            case TRIANGLE_MESH_SHAPE_PROXYTYPE: {
                // For triangle mesh shapes, we can't easily visualize all triangles
                // Just draw a bounding box for now
                btVector3 aabbMin, aabbMax;
                shape->getAabb(trans, aabbMin, aabbMax);
                
                Vector3 center = {
                    (aabbMin.x() + aabbMax.x()) * 0.5f,
                    (aabbMin.y() + aabbMax.y()) * 0.5f,
                    (aabbMin.z() + aabbMax.z()) * 0.5f
                };
                
                Vector3 size = {
                    aabbMax.x() - aabbMin.x(),
                    aabbMax.y() - aabbMin.y(),
                    aabbMax.z() - aabbMin.z()
                };
                
                DrawCubeWires(center, size.x, size.y, size.z, YELLOW);
                break;
            }
            default:
                // Default wireframe visualization
                DrawCubeWires(position, 1.0f, 1.0f, 1.0f, GRAY);
                break;
        }
    }
}