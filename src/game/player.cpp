
#include "player.h"

Player::Player() :
		physicsBody(nullptr), world(nullptr), camera(nullptr) {}

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

void Player::attachCamera(Camera *cam) {
	camera = cam;
	if (camera) {
		Vector3 pos = getPosition();
		camera->position = pos;
		camera->up = (Vector3){ 0.0f, 1.0f, 0.0f };
		camera->fovy = 60.0f;
		camera->projection = CAMERA_PERSPECTIVE;
	}
}

void Player::update(float deltaTime) {
	if (camera) {
		updateCamera();
	}
}

void Player::updateCamera() {
	Vector3 pos = getPosition();
	camera->position = pos;
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