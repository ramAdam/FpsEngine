#pragma once
#include "raylib.h"
#include <btBulletDynamicsCommon.h>

class DebugRenderer
{
public:
    DebugRenderer();
    
    // Basic debug visualization
    void drawGrid();
    void drawAxes();
    
    // Physics debug visualization
    void drawCollisionShapes(btDynamicsWorld* dynamicsWorld);
    void drawTriangleMesh(btTriangleMeshShape* meshShape, const btTransform& worldTransform, Color wireColor);
    void highlightTriangleAt(const Vector3& position, float radius, Color highlightColor, btDynamicsWorld* world);
    
    // Toggle debug visualization
    void toggleGrid() { showGrid = !showGrid; }
    void toggleAxes() { showAxes = !showAxes; }
    void toggleCollisionShapes() { showCollisionShapes = !showCollisionShapes; }
    
    // Check visualization states
    bool isGridVisible() const { return showGrid; }
    bool isAxesVisible() const { return showAxes; }
    bool isCollisionDebugVisible() const { return showCollisionShapes; }
    
private:
    bool showGrid = false;
    bool showAxes = false;
    bool showCollisionShapes = false;

    int grid_slices = 20;
    float grid_spacing = 1.0f;
    bool show_wireframe = true;
};