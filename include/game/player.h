
#pragma once
#include <btBulletDynamicsCommon.h>
#include <raylib.h>

class Player {
public:
	Player();
	~Player();

	void init(btDynamicsWorld *world, const Vector3 &startPos);
	void update(float deltaTime);
	void cleanup();

	// Camera controls
	void attachCamera(Camera *camera);
	void updateCamera();
	bool OnGround();
	void setPosition(const Vector3 &position);
	Vector3 getPosition() const;

private:
	btRigidBody *physicsBody;
	btDynamicsWorld *world;
	Camera *camera;

	void createPhysicsBody(const Vector3 &position);
	static constexpr float PLAYER_HEIGHT = 1.8f;
	static constexpr float PLAYER_RADIUS = 0.5f;
	static constexpr float PLAYER_MASS = 80.0f;
};