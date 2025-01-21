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

	void drawDebugCapsule(bool drawRaycast = true);
	void toggleDebugDraw() { showDebug = !showDebug; }

private:
	void createPhysicsBody(const Vector3 &position);
	void updateCamera();

	// New movement methods
	void handleMovementInput();
	void applyMovement(const Vector3 &direction);
	void handleJump();
	Vector3 calculateMoveDirection();

	btRigidBody *physicsBody;
	btDynamicsWorld *world;
	Camera3D camera;

	float yaw = 0.0f; // Camera rotation around Y axis
	float pitch = 0.0f; // Camera rotation around X axis
	const float mouseSensitivity = 0.003f;

	static constexpr float PLAYER_HEIGHT = 1.8f;
	static constexpr float PLAYER_RADIUS = 0.4f;
	static constexpr float PLAYER_MASS = 1.0f;

	// Movement constants
	static constexpr float MOVE_SPEED = 10.0f;
	static constexpr float JUMP_FORCE = 5.0f;
	static constexpr float AIR_CONTROL = 0.3f; // Movement control in air
	static constexpr float GROUND_DRAG = 0.9f; // Ground movement dampening

	// Movement state
	Vector3 moveDirection;
	Vector3 velocity;
	bool isJumping;
	bool wasOnGround;
	float jumpCooldown;

	bool showDebug = true;
	static constexpr float DEBUG_CAPSULE_SEGMENTS = 12;
	static constexpr Color DEBUG_CAPSULE_COLOR = GREEN;
	static constexpr Color DEBUG_RAYCAST_COLOR = RED;
};