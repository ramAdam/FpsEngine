#include "player.h"

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