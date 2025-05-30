#include "physics_manager.h"
#include <memory>  // Add this for std::make_unique
#include <BulletCollision/CollisionDispatch/btGhostObject.h>

PhysicsManager* PhysicsManager::instance = nullptr;

PhysicsManager::PhysicsManager() : 
    collisionConfiguration(nullptr),
    dispatcher(nullptr),
    overlappingPairCache(nullptr),
    solver(nullptr),
    dynamicsWorld(nullptr),
    groundBody(nullptr)
{
    // Constructor body
}

PhysicsManager::~PhysicsManager()
{
    // Destructor body if needed
    // Note: We removed "= default" from header and provide implementation here
}

PhysicsManager& PhysicsManager::getInstance()
{
    if (instance == nullptr) {
        instance = new PhysicsManager();
    }
    return *instance;
}

void PhysicsManager::init()
{
	collisionConfiguration = std::make_unique<btDefaultCollisionConfiguration>();
	dispatcher = std::make_unique<btCollisionDispatcher>(collisionConfiguration.get());
	overlappingPairCache = std::make_unique<btDbvtBroadphase>();
	solver = std::make_unique<btSequentialImpulseConstraintSolver>();
	dynamicsWorld = std::make_unique<btDiscreteDynamicsWorld>(
		dispatcher.get(),
		overlappingPairCache.get(),
		solver.get(),
		collisionConfiguration.get());

	dynamicsWorld->setGravity(btVector3(0, -9.81f, 0));
	// createGround();
}

void PhysicsManager::createGround()
{
	btCollisionShape *groundShape = new btBoxShape(btVector3(50, 1, 50));
	btTransform groundTransform;
	groundTransform.setIdentity();
	groundTransform.setOrigin(btVector3(0, -1, 0));

	btScalar mass(0.0); // Mass = 0 makes it static
	btVector3 localInertia(0, 0, 0);

	btDefaultMotionState *motionState = new btDefaultMotionState(groundTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, groundShape, localInertia);
	groundBody = new btRigidBody(rbInfo);

	dynamicsWorld->addRigidBody(groundBody);
}

void PhysicsManager::update(float deltaTime)
{
	dynamicsWorld->stepSimulation(deltaTime, 10);
}

void PhysicsManager::cleanup()
{
    // Clean up bodies before destroying the world
	for (auto body : staticBodies)
	{
		if (body) {
			dynamicsWorld->removeRigidBody(body);
			delete body->getMotionState();
			delete body;
		}
	}
	staticBodies.clear();

	// Clean up collision shapes
	for (auto shape : collisionShapes)
	{
		delete shape;
	}
	collisionShapes.clear();
	
	// Clean up collision meshes
	for (auto mesh : collisionMeshes)
	{
		delete mesh;
	}
	collisionMeshes.clear();

	// Clean up ground body
	if (groundBody)
	{
		dynamicsWorld->removeRigidBody(groundBody);
		delete groundBody->getMotionState();
		delete groundBody->getCollisionShape();
		delete groundBody;
		groundBody = nullptr;
	}

	// Clean up ghost objects
	for (auto ghost : ghostObjects)
	{
		if (ghost) {
			dynamicsWorld->removeCollisionObject(ghost);
			// Note: collision shapes are cleaned up in the regular collisionShapes cleanup
			delete ghost;
		}
	}
	ghostObjects.clear();

    // Smart pointers will clean themselves up when PhysicsManager is destroyed
    // Just reset them explicitly if you need to release resources before destruction
    dynamicsWorld.reset();
    solver.reset();
    overlappingPairCache.reset();
    dispatcher.reset();
    collisionConfiguration.reset();
}

void PhysicsManager::createCollisionFromModel(const Model &model)
{
	// Create a triangle mesh for the entire model
	btTriangleMesh *triangleMesh = new btTriangleMesh();

	// Process all meshes in the model
	for (int m = 0; m < model.meshCount; m++)
	{
		// Skip empty meshes
		if (model.meshes[m].vertexCount == 0 || model.meshes[m].triangleCount == 0)
			continue;

		// Get vertices and indices
		float *vertices = model.meshes[m].vertices;
		unsigned short *indices = model.meshes[m].indices;

		// Process all triangles
		for (int i = 0; i < model.meshes[m].triangleCount; i++)
		{
			// Get vertex indices
			int idx1 = indices ? indices[i * 3] : i * 3;
			int idx2 = indices ? indices[i * 3 + 1] : i * 3 + 1;
			int idx3 = indices ? indices[i * 3 + 2] : i * 3 + 2;

			// Get vertex positions
			btVector3 v1(vertices[idx1 * 3], vertices[idx1 * 3 + 1], vertices[idx1 * 3 + 2]);
			btVector3 v2(vertices[idx2 * 3], vertices[idx2 * 3 + 1], vertices[idx2 * 3 + 2]);
			btVector3 v3(vertices[idx3 * 3], vertices[idx3 * 3 + 1], vertices[idx3 * 3 + 2]);

			// Calculate normal to check if it's facing upward
			btVector3 edge1 = v2 - v1;
			btVector3 edge2 = v3 - v1;
			btVector3 normal = edge1.cross(edge2).normalized();

			// Add triangle to mesh
			triangleMesh->addTriangle(v1, v2, v3, true);
		}
	}

	// Create BVH triangle mesh shape for better performance
	btBvhTriangleMeshShape *meshShape = new btBvhTriangleMeshShape(triangleMesh, true);
	
	// Add margin to smooth over tiny irregularities
	meshShape->setMargin(0.05f);

	// Create rigid body
	btTransform transform;
	transform.setIdentity();

	btDefaultMotionState *motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, motionState, meshShape);
	btRigidBody *body = new btRigidBody(rbInfo);

	// Add to world
	dynamicsWorld->addRigidBody(body);

	// Store for cleanup
	collisionShapes.push_back(meshShape);
	collisionMeshes.push_back(triangleMesh);
	staticBodies.push_back(body);
}

btPairCachingGhostObject* PhysicsManager::createGroundSensor(btCollisionObject* parent, float radius)
{
    // Create ghost object
    btPairCachingGhostObject* ghost = new btPairCachingGhostObject();
    
    // Create small capsule shape for ground detection
    btCapsuleShape* shape = new btCapsuleShape(radius, 0.1f);
    shape->setMargin(0.01f);
    
    // Configure ghost object
    ghost->setCollisionShape(shape);
    ghost->setCollisionFlags(btCollisionObject::CF_NO_CONTACT_RESPONSE);
    
    // Set up collision filtering
    int collisionFilterGroup = btBroadphaseProxy::SensorTrigger;
    int collisionFilterMask = btBroadphaseProxy::StaticFilter | btBroadphaseProxy::DefaultFilter;
    
    // Exclude the parent object from collisions
    if (parent) {
        ghost->setUserPointer(parent);
    }
    
    // Add ghost to world
    dynamicsWorld->addCollisionObject(ghost, collisionFilterGroup, collisionFilterMask);
    
    // Store for cleanup
    ghostObjects.push_back(ghost);
    collisionShapes.push_back(shape);
    
    return ghost;
}

void PhysicsManager::updateGroundSensor(btPairCachingGhostObject* sensor, const btVector3& position)
{
    if (!sensor) return;
    
    // Update transform
    btTransform transform;
    transform.setIdentity();
    transform.setOrigin(position);
    sensor->setWorldTransform(transform);
    
    // Update AABB for broad phase
    dynamicsWorld->updateSingleAabb(sensor);
}

bool PhysicsManager::checkGroundContact(btPairCachingGhostObject* sensor)
{
    if (!sensor) return false;
    
    // Check for overlapping objects
    int numOverlapping = sensor->getNumOverlappingObjects();
    
    // Basic check - if anything overlaps, we're on ground
    if (numOverlapping > 0) {
        return true;
    }
    
    return false;
}