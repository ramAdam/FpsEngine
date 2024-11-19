#pragma once
#include <btBulletDynamicsCommon.h>
#include <memory>

class PhysicsManager {
public:
	PhysicsManager();
	~PhysicsManager();

	void init();
	void update(float deltaTime);
	void cleanup();

	void createGround();
	// void createPlayer(const btVector3 &startPos);
	// btVector3 getPlayerPosition() const;
	btDynamicsWorld *getDynamicsWorld() { return dynamicsWorld.get(); }

private:
	std::unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration;
	std::unique_ptr<btCollisionDispatcher> dispatcher;
	std::unique_ptr<btBroadphaseInterface> overlappingPairCache;
	std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
	std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;

	btRigidBody *groundBody;
	// btRigidBody *playerBody;
};