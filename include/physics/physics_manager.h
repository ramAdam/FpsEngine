#pragma once
#include <btBulletDynamicsCommon.h>
#include <BulletCollision/CollisionDispatch/btGhostObject.h>
#include <vector>
#include <memory>  // Add this for std::unique_ptr
#include <raylib.h> // For Model type

class PhysicsManager {
public:
    // Singleton accessor
    static PhysicsManager& getInstance();
    
    // Delete copy constructor and assignment operator
    PhysicsManager(const PhysicsManager&) = delete;
    PhysicsManager& operator=(const PhysicsManager&) = delete;
    
    // Make destructor public
    ~PhysicsManager();
    
    // Initialization and cleanup
    void init();
    void cleanup();
    void update(float deltaTime); // Add this
    
    // Physics world access
    btDynamicsWorld* getDynamicsWorld() { return dynamicsWorld.get(); }
    
    // Ground methods
    void createGround(); // Add this
    void createCollisionFromModel(const Model &model); // Add this
    
    // Ghost object methods for ground detection
    btPairCachingGhostObject* createGroundSensor(btCollisionObject* parent, float radius);
    void updateGroundSensor(btPairCachingGhostObject* sensor, const btVector3& position);
    bool checkGroundContact(btPairCachingGhostObject* sensor);
    
private:
    // Private constructor for singleton
    PhysicsManager();
    
    // Static instance
    static PhysicsManager* instance;
    
    // Bullet physics objects - change to unique_ptr
    std::unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration;
    std::unique_ptr<btCollisionDispatcher> dispatcher;
    std::unique_ptr<btBroadphaseInterface> overlappingPairCache;
    std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
    std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;
    
    // Track objects that need cleanup
    std::vector<btRigidBody*> rigidBodies;
    std::vector<btCollisionShape*> collisionShapes;
    std::vector<btGhostObject*> ghostObjects;
    
    // Ground and static bodies
    btRigidBody* groundBody; // Add this
    std::vector<btRigidBody*> staticBodies; // Add this
    std::vector<btTriangleMesh*> collisionMeshes; // Add this
};