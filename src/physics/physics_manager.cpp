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
	// Clean up static bodies
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