#pragma once
#include <btBulletDynamicsCommon.h>
#include "debug_renderer.h"
#include <raylib.h>
#include "input_manager.h"
#include "ground_detector.h"
#include <memory>

class PhysicsManager;

class Player
{
public:
	Player();
	~Player();

	void init(btDynamicsWorld *dynamicsWorld, const Vector3 &startPos);
	void update(float deltaTime);
	void setPosition(const Vector3 &position);
	Vector3 getPosition() const;
	void cleanup();

	bool OnGround() const;

	const Camera3D &getCamera() const { return camera; }
	void handleMouseInput(float deltaX, float deltaY);
	void initCamera(const Vector3 &position);

	void drawDebugCapsule(bool drawRaycast = true) const;
	void toggleDebugDraw() { showDebug = !showDebug; }
	void drawDebugInfo(DebugRenderer* debugRenderer) const;

private:
	void createPhysicsBody(const Vector3 &position);
	void updateCamera();

	// New movement methods
	void handleMovementInput();
	void applyMovement(const Vector3 &direction);
	void handleJump();
	Vector3 calculateMoveDirection() const;
	bool checkGroundRaycast(); // Checks if player is 1 ground using raycast

	btRigidBody *physicsBody;
	btDynamicsWorld *world;
	Camera3D camera;

	float yaw = 0.0f;	// Camera rotation around Y axis
	float pitch = 0.0f; // Camera rotation around X axis
	const float mouseSensitivity = 0.003f;

	static constexpr float PLAYER_HEIGHT = 1.8f;
	static constexpr float PLAYER_RADIUS = 0.4f;
	static constexpr float PLAYER_MASS = 1.0f;

	// Movement constants
	static constexpr float MOVE_SPEED = 30.0f;  // Increased from 10.0f
	static constexpr float JUMP_FORCE = 5.0f;
	static constexpr float AIR_CONTROL = 0.3f; // Movement control in air
	static constexpr float GROUND_DRAG = 0.9f; // Ground movement dampening

	// Movement state
	Vector3 moveDirection;
	Vector3 velocity;
	bool isJumping;
	bool wasOnGround;
	float jumpCooldown;

	// Added for slope handling
	Vector3 lastGroundNormal;

	// Added variables
	bool wasMovingLastFrame = false;
	Vector3 previousMoveDir = {0, 0, 0};
	Vector3 lastGroundPosition = {0, 0, 0}; 

	// Ground grace period variables
	float lastGroundTime = 0.0f;
	static constexpr float GROUND_GRACE_PERIOD = 0.1f; // 100ms grace period

	// Ground detection
	std::unique_ptr<GroundDetector> groundDetector;

	bool showDebug = true;
	static constexpr float DEBUG_CAPSULE_SEGMENTS = 12;
	static constexpr Color DEBUG_CAPSULE_COLOR = GREEN;
	static constexpr Color DEBUG_RAYCAST_COLOR = RED;

	float raycastDistance = 1.2f;  // For ground detection
};