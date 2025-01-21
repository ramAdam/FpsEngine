#pragma once
#include "raylib.h"
#include <btBulletDynamicsCommon.h>
#include <memory>
#include <vector>

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

	void createCollisionFromModel(const Model &model);
	void addStaticCollisionShape(btCollisionShape *shape, const btVector3 &position);

private:
	std::unique_ptr<btDefaultCollisionConfiguration> collisionConfiguration;
	std::unique_ptr<btCollisionDispatcher> dispatcher;
	std::unique_ptr<btBroadphaseInterface> overlappingPairCache;
	std::unique_ptr<btSequentialImpulseConstraintSolver> solver;
	std::unique_ptr<btDiscreteDynamicsWorld> dynamicsWorld;

	btRigidBody *groundBody;
	// btRigidBody *playerBody;

	std::vector<btRigidBody *> staticBodies; // Store all static collision bodies
	std::vector<btCollisionShape *> collisionShapes; // Store all collision shapes
};