#include "debug_renderer.h"
#include <iostream>

DebugRenderer::DebugRenderer()
{
    // Initialize default values
}

void DebugRenderer::drawGrid()
{
    if (!showGrid) return;
    
    // Draw a grid on the XZ plane
    DrawGrid(grid_slices, grid_spacing);
}

void DebugRenderer::drawAxes()
{
    if (!showAxes) return;
    
    // Draw world axes
    DrawLine3D({0, 0, 0}, {1, 0, 0}, RED);    // X axis
    DrawLine3D({0, 0, 0}, {0, 1, 0}, GREEN);  // Y axis
    DrawLine3D({0, 0, 0}, {0, 0, 1}, BLUE);   // Z axis
}

void DebugRenderer::drawCollisionShapes(btDynamicsWorld* dynamicsWorld)
{
    if (!showCollisionShapes || !dynamicsWorld) return;
    
    // Debug draw all collision objects
    for (int i = 0; i < dynamicsWorld->getNumCollisionObjects(); i++) {
        btCollisionObject* obj = dynamicsWorld->getCollisionObjectArray()[i];
        if (!obj) continue;
        
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
                btBoxShape* boxShape = static_cast<btBoxShape*>(shape);
                btVector3 halfExtents = boxShape->getHalfExtentsWithoutMargin();
                DrawCubeWires(position, halfExtents.x() * 2, halfExtents.y() * 2, halfExtents.z() * 2, color);
                break;
            }
            case SPHERE_SHAPE_PROXYTYPE: {
                btSphereShape* sphereShape = static_cast<btSphereShape*>(shape);
                float radius = sphereShape->getRadius();
                DrawSphereWires(position, radius, 8, 8, color);
                break;
            }
            case CAPSULE_SHAPE_PROXYTYPE: {
                btCapsuleShape* capsuleShape = static_cast<btCapsuleShape*>(shape);
                float radius = capsuleShape->getRadius();
                float halfHeight = capsuleShape->getHalfHeight();
                
                // Draw capsule as a cylinder with two spheres at ends
                Vector3 upDir = {0, 1, 0};
                Vector3 top = {position.x, position.y + halfHeight, position.z};
                Vector3 bottom = {position.x, position.y - halfHeight, position.z};
                
                DrawSphereWires(top, radius, 8, 8, color);
                DrawSphereWires(bottom, radius, 8, 8, color);
                DrawCylinderWires(position, radius, radius, halfHeight * 2, 8, color);
                break;
            }
            case TRIANGLE_MESH_SHAPE_PROXYTYPE:
            case SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE: {
                // Draw bounding box for mesh shape
                btTriangleMeshShape* meshShape = static_cast<btTriangleMeshShape*>(shape);
                
                // Draw bounding box
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
                
                // Draw individual triangles
                Color triangleColor = {color.r, color.g, color.b, 128};  // Semi-transparent
                drawTriangleMesh(meshShape, trans, triangleColor);
                break;
            }
            default:
                // Default wireframe visualization
                DrawCubeWires(position, 1.0f, 1.0f, 1.0f, GRAY);
                break;
        }
    }
}

void DebugRenderer::drawTriangleMesh(btTriangleMeshShape* meshShape, const btTransform& worldTransform, Color wireColor) 
{
    if (!meshShape) return;
    
    // Work with the btStridingMeshInterface
    btStridingMeshInterface* meshInterface = nullptr;
    
    // Handle different types of triangle mesh shapes
    if (meshShape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE) {
        // For btBvhTriangleMeshShape
        btBvhTriangleMeshShape* bvhShape = static_cast<btBvhTriangleMeshShape*>(meshShape);
        meshInterface = bvhShape->getMeshInterface();
    } else if (meshShape->getShapeType() == SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE) {
        // For safety, check shape type first and then cast
        if (meshShape->getShapeType() == SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE) {
            // Use old-style C cast which is more permissive but be careful!
            btScaledBvhTriangleMeshShape* scaledShape = (btScaledBvhTriangleMeshShape*)meshShape;
            btBvhTriangleMeshShape* childShape = (btBvhTriangleMeshShape*)scaledShape->getChildShape();
            if (childShape) {
                meshInterface = childShape->getMeshInterface();
            }
        }
    }
    
    if (!meshInterface) return;
    
    // Get triangle mesh data
    const unsigned char* vertexBase = nullptr;
    int numVerts = 0;
    PHY_ScalarType vertexType;
    int vertexStride = 0;
    const unsigned char* indexBase = nullptr;
    int indexStride = 0;
    int numFaces = 0;
    PHY_ScalarType indexType;
    
    // Use part 0 for simplicity
    const int subpart = 0;
    
    meshInterface->getLockedReadOnlyVertexIndexBase(&vertexBase, numVerts, vertexType, vertexStride, 
                                                   &indexBase, indexStride, numFaces, indexType, subpart);
    
    if (numFaces == 0 || !vertexBase || !indexBase) {
        // Always use the subpart parameter
        meshInterface->unLockReadOnlyVertexBase(subpart);
        return;
    }
    
    // Draw each triangle in the mesh
    for (int i = 0; i < numFaces; i++) {
        // Get vertex indices of the triangle
        int index0, index1, index2;
        
        if (indexType == PHY_INTEGER) {
            // 32-bit indices
            index0 = ((int*)(indexBase + i * indexStride))[0];
            index1 = ((int*)(indexBase + i * indexStride))[1];
            index2 = ((int*)(indexBase + i * indexStride))[2];
        } else {
            // 16-bit indices
            index0 = ((short*)(indexBase + i * indexStride))[0];
            index1 = ((short*)(indexBase + i * indexStride))[1];
            index2 = ((short*)(indexBase + i * indexStride))[2];
        }
        
        // Get vertex positions
        Vector3 v0, v1, v2;
        
        if (vertexType == PHY_FLOAT) {
            // Handle different stride values for float vertices
            const float* verts = (const float*)(vertexBase);
            
            v0 = {
                verts[index0 * (vertexStride/sizeof(float))],
                verts[index0 * (vertexStride/sizeof(float)) + 1],
                verts[index0 * (vertexStride/sizeof(float)) + 2]
            };
            
            v1 = {
                verts[index1 * (vertexStride/sizeof(float))],
                verts[index1 * (vertexStride/sizeof(float)) + 1],
                verts[index1 * (vertexStride/sizeof(float)) + 2]
            };
            
            v2 = {
                verts[index2 * (vertexStride/sizeof(float))],
                verts[index2 * (vertexStride/sizeof(float)) + 1],
                verts[index2 * (vertexStride/sizeof(float)) + 2]
            };
        } else if (vertexType == PHY_DOUBLE) {
            // Handle double precision vertices
            const double* verts = (const double*)(vertexBase);
            
            v0 = {
                (float)verts[index0 * (vertexStride/sizeof(double))],
                (float)verts[index0 * (vertexStride/sizeof(double)) + 1],
                (float)verts[index0 * (vertexStride/sizeof(double)) + 2]
            };
            
            v1 = {
                (float)verts[index1 * (vertexStride/sizeof(double))],
                (float)verts[index1 * (vertexStride/sizeof(double)) + 1],
                (float)verts[index1 * (vertexStride/sizeof(double)) + 2]
            };
            
            v2 = {
                (float)verts[index2 * (vertexStride/sizeof(double))],
                (float)verts[index2 * (vertexStride/sizeof(double)) + 1],
                (float)verts[index2 * (vertexStride/sizeof(double)) + 2]
            };
        }
        
        // Transform vertices by world transform
        btVector3 bt_v0(v0.x, v0.y, v0.z);
        btVector3 bt_v1(v1.x, v1.y, v1.z);
        btVector3 bt_v2(v2.x, v2.y, v2.z);
        
        bt_v0 = worldTransform * bt_v0;
        bt_v1 = worldTransform * bt_v1;
        bt_v2 = worldTransform * bt_v2;
        
        v0 = { bt_v0.x(), bt_v0.y(), bt_v0.z() };
        v1 = { bt_v1.x(), bt_v1.y(), bt_v1.z() };
        v2 = { bt_v2.x(), bt_v2.y(), bt_v2.z() };
        
        // Draw the triangle wireframe
        DrawLine3D(v0, v1, wireColor);
        DrawLine3D(v1, v2, wireColor);
        DrawLine3D(v2, v0, wireColor);
    }
    
    // Unlock vertex buffer WITH the subpart parameter
    meshInterface->unLockReadOnlyVertexBase(subpart);
}

void DebugRenderer::highlightTriangleAt(const Vector3& position, float radius, Color highlightColor, btDynamicsWorld* world) 
{
    if (!world) return;
    
    // Cast a ray downward from position
    btVector3 from(position.x, position.y + 0.1f, position.z);
    btVector3 to(position.x, position.y - radius, position.z);
    
    btCollisionWorld::ClosestRayResultCallback rayCallback(from, to);
    
    // Perform raycast
    world->rayTest(from, to, rayCallback);
    
    if (rayCallback.hasHit()) {
        // Get the hit object and shape
        btCollisionObject* hitObject = const_cast<btCollisionObject*>(rayCallback.m_collisionObject);
        btCollisionShape* hitShape = hitObject->getCollisionShape();
        
        // Check if it's a triangle mesh
        if (hitShape->getShapeType() == TRIANGLE_MESH_SHAPE_PROXYTYPE || 
            hitShape->getShapeType() == SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE) {
            
            btTransform hitTransform = hitObject->getWorldTransform();
            
            // Draw a circle at the hit point
            btVector3 hitPoint = rayCallback.m_hitPointWorld;
            Vector3 hitPos = {hitPoint.x(), hitPoint.y(), hitPoint.z()};
            
            // Draw hit point marker
            DrawSphere(hitPos, 0.1f, highlightColor);
            
            // Draw hit normal
            btVector3 hitNormal = rayCallback.m_hitNormalWorld;
            Vector3 normal = {hitNormal.x(), hitNormal.y(), hitNormal.z()};
            Vector3 normalEnd = {
                hitPos.x + normal.x * 0.5f,
                hitPos.y + normal.y * 0.5f,
                hitPos.z + normal.z * 0.5f
            };
            
            DrawLine3D(hitPos, normalEnd, RED);
        }
    }
}