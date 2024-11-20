#pragma once
#include <btBulletDynamicsCommon.h>
#include <raylib.h>

class Player {
public:
	Player();
	~Player();

	void init(btDynamicsWorld *dynamicsWorld, const Vector3 &startPos);
	void update(float deltaTime);
	void setPosition(const Vector3 &position);
	Vector3 getPosition() const;
	void cleanup();

	bool OnGround();

	const Camera3D &getCamera() const { return camera; }
	void handleMouseInput(float deltaX, float deltaY);
	void initCamera(const Vector3 &position);

private:
	void createPhysicsBody(const Vector3 &position);
	void updateCamera();

	btRigidBody *physicsBody;
	btDynamicsWorld *world;
	Camera3D camera;

	float yaw = 0.0f; // Camera rotation around Y axis
	float pitch = 0.0f; // Camera rotation around X axis
	const float mouseSensitivity = 0.003f;

	static constexpr float PLAYER_HEIGHT = 2.0f;
	static constexpr float PLAYER_RADIUS = 0.5f;
	static constexpr float PLAYER_MASS = 1.0f;

	// Movement constants
	static constexpr float MOVE_SPEED = 10.0f;
	static constexpr float JUMP_FORCE = 5.0f;

	// Movement state
	Vector3 moveDirection;
	bool isJumping;
};