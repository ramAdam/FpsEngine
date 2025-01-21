#include "physics_manager.h"

PhysicsManager::PhysicsManager() : groundBody(nullptr) {}

PhysicsManager::~PhysicsManager()
{
	cleanup();
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
	createGround();
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
	// Clean up static bodies
	for (auto body : staticBodies)
	{
		delete body->getMotionState();
		delete body;
	}
	staticBodies.clear();

	// Clean up collision shapes
	for (auto shape : collisionShapes)
	{
		delete shape;
	}
	collisionShapes.clear();

	// Clean up ground body
	if (groundBody)
	{
		dynamicsWorld->removeRigidBody(groundBody);
		delete groundBody->getMotionState();
		delete groundBody->getCollisionShape();
		delete groundBody;
		groundBody = nullptr;
	}
}

void PhysicsManager::createCollisionFromModel(const Model &model)
{
	// Create a simple box collision shape based on model bounds
	BoundingBox bounds = GetMeshBoundingBox(model.meshes[0]);

	// Calculate box dimensions
	btVector3 boxExtents(
		(bounds.max.x - bounds.min.x) * 0.5f,
		(bounds.max.y - bounds.min.y) * 0.5f,
		(bounds.max.z - bounds.min.z) * 0.5f);

	// Create box shape
	btBoxShape *boxShape = new btBoxShape(boxExtents);

	// Calculate center position
	btVector3 position(
		(bounds.max.x + bounds.min.x) * 0.5f,
		(bounds.max.y + bounds.min.y) * 0.5f,
		(bounds.max.z + bounds.min.z) * 0.5f);

	// Add to physics world
	addStaticCollisionShape(boxShape, position);
	collisionShapes.push_back(boxShape);
}

void PhysicsManager::addStaticCollisionShape(btCollisionShape *shape, const btVector3 &position)
{
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(position);

	btDefaultMotionState *motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, motionState, shape); // Mass = 0 for static body
	btRigidBody *body = new btRigidBody(rbInfo);

	dynamicsWorld->addRigidBody(body);
	staticBodies.push_back(body);
}