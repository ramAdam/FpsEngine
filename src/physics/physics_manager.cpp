#include "physics_manager.h"

PhysicsManager::PhysicsManager() :
		groundBody(nullptr) {}

PhysicsManager::~PhysicsManager() {
	cleanup();
}

void PhysicsManager::init() {
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

void PhysicsManager::createGround() {
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

void PhysicsManager::update(float deltaTime) {
	dynamicsWorld->stepSimulation(deltaTime, 10);
}

void PhysicsManager::cleanup() {
	if (dynamicsWorld) {
		// Clean up static bodies
		for (auto body : staticBodies) {
			dynamicsWorld->removeRigidBody(body);
			delete body->getMotionState();
			delete body;
		}
		staticBodies.clear();

		// Clean up collision shapes
		for (auto shape : collisionShapes) {
			delete shape;
		}
		collisionShapes.clear();

		if (groundBody) {
			dynamicsWorld->removeRigidBody(groundBody);
			delete groundBody->getMotionState();
			delete groundBody->getCollisionShape();
			delete groundBody;
		}
	}
}

void PhysicsManager::createCollisionFromModel(const Model &model) {
	// Process each mesh in the model
	for (int i = 0; i < model.meshCount; i++) {
		const Mesh &mesh = model.meshes[i];

		// Create triangle mesh
		btTriangleMesh *triangleMesh = new btTriangleMesh();

		// Add all triangles to the mesh
		for (int j = 0; j < mesh.vertexCount; j += 3) {
			btVector3 vertex1(mesh.vertices[j * 3], mesh.vertices[j * 3 + 1], mesh.vertices[j * 3 + 2]);
			btVector3 vertex2(mesh.vertices[(j + 1) * 3], mesh.vertices[(j + 1) * 3 + 1], mesh.vertices[(j + 2) * 3 + 2]);
			btVector3 vertex3(mesh.vertices[(j + 2) * 3], mesh.vertices[(j + 2) * 3 + 1], mesh.vertices[(j + 2) * 3 + 2]);
			triangleMesh->addTriangle(vertex1, vertex2, vertex3);
		}

		// Create collision shape from triangle mesh
		btBvhTriangleMeshShape *meshShape = new btBvhTriangleMeshShape(triangleMesh, true);
		addStaticCollisionShape(meshShape, btVector3(0, 0, 0));
		collisionShapes.push_back(meshShape);
	}
}

void PhysicsManager::addStaticCollisionShape(btCollisionShape *shape, const btVector3 &position) {
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(position);

	btDefaultMotionState *motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, motionState, shape); // Mass = 0 for static body
	btRigidBody *body = new btRigidBody(rbInfo);

	dynamicsWorld->addRigidBody(body);
	staticBodies.push_back(body);
}