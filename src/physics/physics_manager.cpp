
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

// void PhysicsManager::createPlayer(const btVector3 &startPos) {
// 	btCollisionShape *capsule = new btCapsuleShape(0.5f, 1.0f);
// 	btTransform startTransform;
// 	startTransform.setIdentity();
// 	startTransform.setOrigin(startPos);

// 	btScalar mass(80.0); // 80 kg player
// 	btVector3 localInertia(0, 0, 0);
// 	capsule->calculateLocalInertia(mass, localInertia);

// 	btDefaultMotionState *motionState = new btDefaultMotionState(startTransform);
// 	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, capsule, localInertia);
// 	playerBody = new btRigidBody(rbInfo);

// 	playerBody->setAngularFactor(btVector3(0, 1, 0)); // Only allow rotation around Y axis
// 	playerBody->setActivationState(DISABLE_DEACTIVATION);

// 	dynamicsWorld->addRigidBody(playerBody);
// }

void PhysicsManager::update(float deltaTime) {
	dynamicsWorld->stepSimulation(deltaTime, 10);
}

// btVector3 PhysicsManager::getPlayerPosition() const {
// 	if (!playerBody)
// 		return btVector3(0, 0, 0);
// 	btTransform trans;
// 	playerBody->getMotionState()->getWorldTransform(trans);
// 	return trans.getOrigin();
// }

void PhysicsManager::cleanup() {
	if (dynamicsWorld) {
		// if (playerBody) {
		// 	dynamicsWorld->removeRigidBody(playerBody);
		// 	delete playerBody->getMotionState();
		// 	delete playerBody->getCollisionShape();
		// 	delete playerBody;
		// }
		if (groundBody) {
			dynamicsWorld->removeRigidBody(groundBody);
			delete groundBody->getMotionState();
			delete groundBody->getCollisionShape();
			delete groundBody;
		}
	}
}