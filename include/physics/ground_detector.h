#pragma once
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

// Forward declaration
class PhysicsManager;
class DebugRenderer;

class GroundDetector {
public:
    GroundDetector(PhysicsManager& physics, btRigidBody* parentBody, float radius = 0.4f);
    ~GroundDetector();
    
    // Core functionality
    void update(const btVector3& feetPosition);
    bool isOnGround() const;
    
    // Advanced ground info
    btVector3 getGroundNormal() const { return groundNormal; }
    float getGroundDistance() const { return groundDistance; }
    
    // Ground grace period
    void setGracePeriod(float seconds) { gracePeriod = seconds; }
    float getGracePeriod() const { return gracePeriod; }
    
    // Debug visualization
    void drawDebug(DebugRenderer* renderer) const;
    
private:
    // Detect ground using ghost object
    bool detectGroundGhost();
    
    // Detect ground using raycast
    bool detectGroundRaycast();
    
    // Ground normal calculation
    bool calculateGroundNormal();
    
    // Data
    PhysicsManager& physics;  // Change to reference
    btRigidBody* parentBody;
    btPairCachingGhostObject* groundSensor;
    
    // Ground state
    bool onGround;
    float groundDistance;
    btVector3 groundNormal;
    btVector3 groundPoint;
    
    // Ground grace period
    float gracePeriod;
    float timeLeftInAir;
    
    // Settings
    float sensorRadius;
    float raycastDistance;
};