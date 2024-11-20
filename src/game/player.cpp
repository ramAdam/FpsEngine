#include "player.h"
#include <raymath.h>
#include <iostream>

Player::Player() :
		physicsBody(nullptr),
		world(nullptr),
		yaw(0.0f),
		pitch(0.0f),
		mouseSensitivity(0.003f) {
	initCamera({ 0, 0, 0 });
}

Player::~Player() {
	cleanup();
}

void Player::init(btDynamicsWorld *dynamicsWorld, const Vector3 &startPos) {
	world = dynamicsWorld;
	createPhysicsBody(startPos);
}

void Player::createPhysicsBody(const Vector3 &position) {
	btCollisionShape *capsule = new btCapsuleShape(PLAYER_RADIUS, PLAYER_HEIGHT);
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(position.x, position.y, position.z));

	btVector3 localInertia(0, 0, 0);
	capsule->calculateLocalInertia(PLAYER_MASS, localInertia);

	btDefaultMotionState *motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(PLAYER_MASS, motionState, capsule, localInertia);
	physicsBody = new btRigidBody(rbInfo);

	physicsBody->setAngularFactor(btVector3(0, 1, 0));
	physicsBody->setActivationState(DISABLE_DEACTIVATION);

	world->addRigidBody(physicsBody);
}

void Player::initCamera(const Vector3 &position) {
	camera.position = position;
	camera.target = { position.x, position.y, position.z + 1.0f };
	camera.up = { 0.0f, 1.0f, 0.0f };
	camera.fovy = 60.0f;
	camera.projection = CAMERA_PERSPECTIVE;
}

void Player::update(float deltaTime) {
	// Handle movement input
	moveDirection = { 0, 0, 0 };

	// Forward/Backward
	if (IsKeyDown(KEY_W))
		moveDirection.z = 1.0f;
	if (IsKeyDown(KEY_S))
		moveDirection.z = -1.0f;

	// Left/Right
	if (IsKeyDown(KEY_A))
		moveDirection.x = -1.0f;
	if (IsKeyDown(KEY_D))
		moveDirection.x = 1.0f;

	// Jump
	if (IsKeyPressed(KEY_SPACE) && OnGround()) {
		physicsBody->applyCentralImpulse(btVector3(0, JUMP_FORCE, 0));
	}

	// Calculate movement vector based on camera direction
	Vector3 forward = Vector3Subtract(camera.target, camera.position);
	forward.y = 0; // Keep movement horizontal
	forward = Vector3Normalize(forward);

	Vector3 right = Vector3CrossProduct(forward, { 0, 1, 0 });

	// Combine movement
	Vector3 finalMove = { 0 };
	if (moveDirection.x != 0 || moveDirection.z != 0) {
		finalMove = Vector3Add(
				Vector3Scale(right, moveDirection.x),
				Vector3Scale(forward, moveDirection.z));
		finalMove = Vector3Scale(Vector3Normalize(finalMove), MOVE_SPEED);
	}

	// Apply movement force
	if (physicsBody) {
		btVector3 velocity = physicsBody->getLinearVelocity();
		btVector3 horizontalVel(finalMove.x, 0, finalMove.z);
		physicsBody->setLinearVelocity(btVector3(horizontalVel.x(), velocity.y(), horizontalVel.z()));
	}

	updateCamera();
}

void Player::handleMouseInput(float deltaX, float deltaY) {
	yaw += deltaX * mouseSensitivity;
	pitch -= deltaY * mouseSensitivity;

	// Clamp pitch to avoid camera flipping
	if (pitch > 1.5f)
		pitch = 1.5f;
	if (pitch < -1.5f)
		pitch = -1.5f;
}

void Player::updateCamera() {
	Vector3 pos = getPosition();
	camera.position = pos;

	// Calculate camera target based on rotation
	float dx = cos(pitch) * cos(yaw);
	float dy = sin(pitch);
	float dz = cos(pitch) * sin(yaw);

	camera.target = {
		pos.x + dx,
		pos.y + dy,
		pos.z + dz
	};
}

bool Player::OnGround() {
	if (!physicsBody || !world)
		return false;

	std::cout << "Checking for ground..." << std::endl;

	btTransform transform = physicsBody->getWorldTransform();
	btVector3 from = transform.getOrigin();

	// Cast ray from center of capsule to slightly below feet
	// Account for capsule height and add a small threshold
	float rayLength = PLAYER_HEIGHT / 2 + 0.5f; // Slightly longer than half height
	btVector3 to = from - btVector3(0, rayLength, 0);

	btCollisionWorld::ClosestRayResultCallback rayCallback(from, to);

	// Ignore collisions with the player's own collision shape
	rayCallback.m_collisionFilterMask = ~rayCallback.m_collisionFilterGroup;

	world->rayTest(from, to, rayCallback);

	// For debugging
	if (rayCallback.hasHit()) {
		std::cout << "Ground detected! Distance: " << rayCallback.m_closestHitFraction * rayLength << std::endl;
	}

	return rayCallback.hasHit();
}

void Player::setPosition(const Vector3 &position) {
	if (!physicsBody)
		return;

	btTransform transform;
	physicsBody->getMotionState()->getWorldTransform(transform);
	transform.setOrigin(btVector3(position.x, position.y, position.z));
	physicsBody->setWorldTransform(transform);
	physicsBody->getMotionState()->setWorldTransform(transform);
	physicsBody->clearForces();
	physicsBody->setLinearVelocity(btVector3(0, 0, 0));
	physicsBody->setAngularVelocity(btVector3(0, 0, 0));
}

Vector3 Player::getPosition() const {
	if (!physicsBody)
		return (Vector3){ 0, 0, 0 };

	btTransform transform;
	physicsBody->getMotionState()->getWorldTransform(transform);
	btVector3 pos = transform.getOrigin();
	return (Vector3){ pos.x(), pos.y(), pos.z() };
}

void Player::cleanup() {
	if (world && physicsBody) {
		world->removeRigidBody(physicsBody);
		delete physicsBody->getMotionState();
		delete physicsBody->getCollisionShape();
		delete physicsBody;
		physicsBody = nullptr;
	}
}