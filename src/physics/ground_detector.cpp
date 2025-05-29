#include "ground_detector.h"
#include "physics_manager.h"
#include "debug_renderer.h"
#include <iostream>

GroundDetector::GroundDetector(PhysicsManager& physics, btRigidBody* parentBody, float radius)
    : physics(physics),
      parentBody(parentBody),
      groundSensor(nullptr),
      onGround(false),
      groundDistance(999.0f),
      groundNormal(0, 1, 0),
      gracePeriod(0.1f),
      timeLeftInAir(0),
      sensorRadius(radius),
      raycastDistance(0.3f)
{
    if (parentBody) {
        groundSensor = physics.createGroundSensor(parentBody, sensorRadius);
    }
}

GroundDetector::~GroundDetector()
{
    // PhysicsManager handles cleanup of the ghost object
}

void GroundDetector::update(const btVector3& feetPosition)
{
    // Update ghost sensor position
    if (groundSensor) {  // No need to check references - they're always valid
        // Position slightly below feet for better detection
        btVector3 sensorPos = feetPosition - btVector3(0, 0.05f, 0);
        
        // Update the sensor position in the physics manager
        physics.updateGroundSensor(groundSensor, sensorPos);
        
        // Check for ground contact using ghost
        bool ghostContact = detectGroundGhost();
        
        // Check for ground contact using raycast (more precise for distance)
        bool rayContact = detectGroundRaycast();
        
        // Calculate ground normal if either contact method succeeded
        if (ghostContact || rayContact) {
            calculateGroundNormal();
        }
        
        // Consider on ground if either method detects ground
        bool actuallyOnGround = ghostContact || rayContact;
        
        // Apply grace period
        if (actuallyOnGround) {
            timeLeftInAir = gracePeriod;
            onGround = true;
        } else if (timeLeftInAir > 0) {
            // Still considered on ground during grace period
            timeLeftInAir -= 1.0f/60.0f; // Assuming 60fps for simplicity
            onGround = true;
        } else {
            onGround = false;
        }
    }
}

bool GroundDetector::isOnGround() const
{
    return onGround;
}

bool GroundDetector::detectGroundGhost()
{
    if (!groundSensor) return false;
    
    bool contact = physics.checkGroundContact(groundSensor);
    return contact;
}

bool GroundDetector::detectGroundRaycast()
{
    if (!parentBody) return false;
    
    // Get current position
    btTransform transform = parentBody->getWorldTransform();
    btVector3 origin = transform.getOrigin();
    
    // Cast ray downward
    btVector3 from = origin;
    btVector3 to = from - btVector3(0, raycastDistance, 0);
    
    // Use bullet's built-in rayTest
    btCollisionWorld* world = physics.getDynamicsWorld();
    btCollisionWorld::ClosestRayResultCallback rayCallback(from, to);
    
    // Filter out the parent body itself
    rayCallback.m_collisionFilterGroup = btBroadphaseProxy::DefaultFilter;
    rayCallback.m_collisionFilterMask = btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::CharacterFilter;
    
    world->rayTest(from, to, rayCallback);
    
    if (rayCallback.hasHit()) {
        // Store ground distance
        groundDistance = rayCallback.m_closestHitFraction * raycastDistance;
        
        // Store ground position
        groundPoint = rayCallback.m_hitPointWorld;
        
        // Store ground normal
        groundNormal = rayCallback.m_hitNormalWorld;
        
        return true;
    }
    
    // No hit
    groundDistance = raycastDistance;
    return false;
}

bool GroundDetector::calculateGroundNormal()
{
    // This is a placeholder to add more sophisticated ground normal calculation
    // if needed (for example, by sampling multiple contact points)
    
    // Currently, the normal is already stored from the raycast
    return true;
}

void GroundDetector::drawDebug(DebugRenderer* renderer) const
{
    if (!renderer) return;
    
    // Get entity position
    btVector3 pos;
    if (parentBody) {
        pos = parentBody->getWorldTransform().getOrigin();
    }
    
    // Convert to raylib Vector3
    Vector3 position = {pos.x(), pos.y(), pos.z()};
    
    // Draw ground sensor area
    if (groundSensor) {
        btTransform sensorTransform = groundSensor->getWorldTransform();
        btVector3 sensorPos = sensorTransform.getOrigin();
        
        // Convert to raylib format
        Vector3 sensorPosition = {sensorPos.x(), sensorPos.y(), sensorPos.z()};
        
        // Draw sensor as wireframe sphere
        Color sensorColor = onGround ? GREEN : RED;
        DrawSphereWires(sensorPosition, sensorRadius, 8, 8, sensorColor);
        
        // Draw ground normal
        if (onGround) {
            Vector3 normalStart = {groundPoint.x(), groundPoint.y(), groundPoint.z()};
            Vector3 normalEnd = {
                normalStart.x + groundNormal.x() * 0.5f,
                normalStart.y + groundNormal.y() * 0.5f,
                normalStart.z + groundNormal.z() * 0.5f
            };
            
            DrawLine3D(normalStart, normalEnd, BLUE);
        }
    }
}