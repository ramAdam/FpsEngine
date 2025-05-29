#include "player.h"
#include <raymath.h>
#include <iostream>

#define GROUND_DRAG 0.002f  // Was 0.9f

Player::Player() : physicsBody(nullptr),
				   world(nullptr),
				   yaw(0.0f),
				   pitch(0.0f),
				   mouseSensitivity(0.003f),
				   isJumping(false),
				   wasOnGround(false),
				   jumpCooldown(0)
{
	initCamera({0, 0, 0});
}

Player::~Player()
{
	cleanup();
}

void Player::init(btDynamicsWorld *dynamicsWorld, const Vector3 &startPos)
{
	world = dynamicsWorld;
	createPhysicsBody(startPos);
	// Remove InputManager initialization
}

void Player::createPhysicsBody(const Vector3 &position)
{
	btCollisionShape *capsule = new btCapsuleShape(PLAYER_RADIUS, PLAYER_HEIGHT);
	btTransform transform;
	transform.setIdentity();
	transform.setOrigin(btVector3(position.x, position.y + PLAYER_HEIGHT / 2, position.z));

	btVector3 localInertia(0, 0, 0);
	capsule->calculateLocalInertia(PLAYER_MASS, localInertia);

	btDefaultMotionState *motionState = new btDefaultMotionState(transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(PLAYER_MASS, motionState, capsule, localInertia);
	physicsBody = new btRigidBody(rbInfo);

	physicsBody->setAngularFactor(btVector3(0, 1, 0));
	physicsBody->setActivationState(DISABLE_DEACTIVATION);

	// Add these parameters for smoother collision
	physicsBody->setContactProcessingThreshold(0.025f);
	physicsBody->setFriction(0.1f);  // Lower friction for smoother sliding

	world->addRigidBody(physicsBody);
}

void Player::initCamera(const Vector3 &position)
{
	camera.position = position;
	camera.target = {position.x, position.y, position.z + 1.0f};
	camera.up = {0.0f, 1.0f, 0.0f};
	camera.fovy = 60.0f;
	camera.projection = CAMERA_PERSPECTIVE;
}

void Player::update(float deltaTime)
{
	handleMovementInput();
	handleJump();

	Vector3 moveDir = calculateMoveDirection();
	
	// Add debugging for wasMovingLastFrame - PUT IT HERE
	std::cout << "wasMovingLastFrame: " << wasMovingLastFrame << std::endl;
	
	// Use your existing OnGround() method instead
	bool onGround = OnGround();  // This method works better
	
	// Debug output
	std::cout << "hasInput: " << (moveDir.x != 0 || moveDir.z != 0)
			  << ", onGround: " << onGround 
			  << ", pos.y: " << getPosition().y << std::endl;
			  
	// Enhanced button release detection
	bool hasMovementInput = (moveDir.x != 0.0f || moveDir.z != 0.0f);
	bool justStopped = wasMovingLastFrame && !hasMovementInput;
	
	// Debug output
	std::cout << "hasInput: " << hasMovementInput 
			  << ", justStopped: " << justStopped
			  << ", onGround: " << onGround << std::endl;
	
	// TWO IMPORTANT FIXES:
	// 1. Make sure we're initialized
	if (wasMovingLastFrame == false && previousMoveDir.x == 0 && previousMoveDir.z == 0) {
		wasMovingLastFrame = hasMovementInput;
	}
	
	// 2. Apply a hard stop when buttons are just released
	if (justStopped) {
		std::cout << "HARD STOP ACTIVATED!" << std::endl;
		
		if (onGround) {
			// Immediate complete stop on button release when on ground
			btVector3 vel = physicsBody->getLinearVelocity();
			vel.setX(0);
			vel.setZ(0);
			physicsBody->setLinearVelocity(vel);
		}
	}
	
	// Only apply movement code if we have input
	if (hasMovementInput) {
		// Apply different movement characteristics based on ground state
		float currentSpeed = onGround ? MOVE_SPEED : (MOVE_SPEED * AIR_CONTROL);

		// Calculate target velocity based on movement direction
		Vector3 targetVelocity = {
			moveDir.x * currentSpeed,
			0,
			moveDir.z * currentSpeed
		};
		
		// On ground, use almost instant response, in air use smoothing
		if (onGround) {
			// Direct control on ground - NO smoothing
			btVector3 newVelocity(
				targetVelocity.x,
				physicsBody->getLinearVelocity().y(),
				targetVelocity.z
			);
			physicsBody->setLinearVelocity(newVelocity);
		} else {
			// In air, apply more limited control with smoothing
			btVector3 currentVel = physicsBody->getLinearVelocity();
			Vector3 currentVelocity = {currentVel.x(), 0, currentVel.z()};
			
			// Air control smoothing - gradual changes
			float airSmoothFactor = 0.15f;
			Vector3 smoothedVelocity = {
				currentVelocity.x + (targetVelocity.x - currentVelocity.x) * airSmoothFactor,
				0,
				currentVelocity.z + (targetVelocity.z - currentVelocity.z) * airSmoothFactor
			};
			
			btVector3 newVelocity(
				smoothedVelocity.x,
				physicsBody->getLinearVelocity().y(),
				smoothedVelocity.z
			);
			physicsBody->setLinearVelocity(newVelocity);
		}
	} else if (onGround) {
		// Add a continuous deceleration for no-input
		if (!hasMovementInput && onGround) {
			// Get current velocity
			btVector3 vel = physicsBody->getLinearVelocity();
			float speed = sqrt(vel.x()*vel.x() + vel.z()*vel.z());
			
			// Apply strong deceleration or immediate stop
			if (speed < 1.0f) {
				// Below threshold, stop completely
				vel.setX(0);
				vel.setZ(0);
			} else {
				// Apply very strong friction (much stronger than before)
				vel.setX(vel.x() * 0.5f);  // Was 0.75f - make more aggressive
				vel.setZ(vel.z() * 0.5f);
			}
			physicsBody->setLinearVelocity(vel);
		}
	}

	// IMPORTANT: Update tracking state at the end
	wasMovingLastFrame = hasMovementInput;
	previousMoveDir = moveDir;
	
	// Update jump state
	wasOnGround = onGround;
	if (jumpCooldown > 0)
		jumpCooldown -= deltaTime;

	updateCamera();
}



void Player::handleMovementInput()
{
	moveDirection = {0, 0, 0};

	auto &input = InputManager::getInstance();
	if (input.isActionPressed(InputAction::MOVE_FORWARD))
		moveDirection.z = 1.0f;
	if (input.isActionPressed(InputAction::MOVE_BACKWARD))
		moveDirection.z = -1.0f;
	if (input.isActionPressed(InputAction::MOVE_LEFT))
		moveDirection.x = -1.0f;
	if (input.isActionPressed(InputAction::MOVE_RIGHT))
		moveDirection.x = 1.0f;
}

void Player::handleJump()
{
	if (!physicsBody)
		return;

	bool onGround = OnGround();
	if (InputManager::getInstance().isActionJustPressed(InputAction::JUMP) && onGround && jumpCooldown <= 0)
	{
		physicsBody->setLinearVelocity(btVector3(
			physicsBody->getLinearVelocity().x(),
			0, // Reset vertical velocity before jump
			physicsBody->getLinearVelocity().z()));
		physicsBody->applyCentralImpulse(btVector3(0, JUMP_FORCE, 0));
		isJumping = true;
		jumpCooldown = 0.1f; // Prevent jump spam
	}
}

Vector3 Player::calculateMoveDirection()
{
	if (moveDirection.x == 0 && moveDirection.z == 0)
	{
		return {0, 0, 0};
	}

	// Get forward and right vectors from camera
	Vector3 forward = Vector3Subtract(camera.target, camera.position);
	forward.y = 0; // Keep movement horizontal
	forward = Vector3Normalize(forward);
	Vector3 right = Vector3CrossProduct(forward, {0, 1, 0});

	// Combine movement
	Vector3 finalMove = Vector3Add(
		Vector3Scale(right, moveDirection.x),
		Vector3Scale(forward, moveDirection.z));

	return Vector3Normalize(finalMove);
}

void Player::applyMovement(const Vector3 &direction)
{
	if (!physicsBody)
		return;

	btVector3 velocity = physicsBody->getLinearVelocity();
	btVector3 horizontalVel(direction.x, 0, direction.z);
	physicsBody->setLinearVelocity(btVector3(horizontalVel.x(), velocity.y(), horizontalVel.z()));
}

void Player::handleMouseInput(float deltaX, float deltaY)
{
	yaw += deltaX * mouseSensitivity;
	pitch -= deltaY * mouseSensitivity;

	// Clamp pitch to avoid camera flipping
	if (pitch > 1.5f)
		pitch = 1.5f;
	if (pitch < -1.5f)
		pitch = -1.5f;
}

void Player::updateCamera()
{
	Vector3 pos = getPosition();
	camera.position = pos;

	// Calculate camera target based on rotation
	float dx = cos(pitch) * cos(yaw);
	float dy = sin(pitch);
	float dz = cos(pitch) * sin(yaw);

	camera.target = {
		pos.x + dx,
		pos.y + dy,
		pos.z + dz};
}

bool Player::OnGround()
{
	if (!physicsBody || !world)
		return false;

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
	// if (rayCallback.hasHit()) {
	// 	std::cout << "Ground detected! Distance: " << rayCallback.m_closestHitFraction * rayLength << std::endl;
	// }

	return rayCallback.hasHit();
}

bool Player::checkGroundRaycast()
{
    if (!physicsBody || !world) return false;
    
    // Get current position (center of capsule)
    btTransform transform = physicsBody->getWorldTransform();
    btVector3 center = transform.getOrigin();
    
    // Calculate position at bottom of capsule
    btVector3 from = center - btVector3(0, PLAYER_HEIGHT/2, 0);
    
    // Add debug output to see ray origin
    std::cout << "Ray origin: " << from.y() << std::endl;
    
    // Cast ray downward with a small distance
    float checkDistance = 0.3f;  // Small distance below feet
    btVector3 to = from - btVector3(0, checkDistance, 0);
    
    // Perform raycast
    btCollisionWorld::ClosestRayResultCallback rayCallback(from, to);
    
    // Exclude player's own collision object
    rayCallback.m_collisionFilterGroup = btBroadphaseProxy::DefaultFilter;
    rayCallback.m_collisionFilterMask = btBroadphaseProxy::AllFilter & ~btBroadphaseProxy::CharacterFilter;
    
    // Perform raycast
    world->rayTest(from, to, rayCallback);
    
    // Add debug output
    std::cout << "Ground check: " << (rayCallback.hasHit() ? "HIT" : "MISS") << std::endl;
    
    // Check if ray hit something
    if (rayCallback.hasHit())
    {
        // Calculate distance to ground
        float distance = (rayCallback.m_hitPointWorld - from).length();
        
        // Store hit position for slope calculations
        lastGroundPosition = {
            rayCallback.m_hitPointWorld.x(),
            rayCallback.m_hitPointWorld.y(),
            rayCallback.m_hitPointWorld.z()
        };
        
        return true;  // We're directly testing a short distance, so any hit means ground
    }
    
    return false;
}

void Player::setPosition(const Vector3 &position)
{
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

Vector3 Player::getPosition() const
{
	if (!physicsBody)
		return (Vector3){0, 0, 0};

	btTransform transform;
	physicsBody->getMotionState()->getWorldTransform(transform);
	btVector3 pos = transform.getOrigin();
	return (Vector3){pos.x(), pos.y(), pos.z()};
}

void Player::cleanup()
{
	// Remove InputManager cleanup
	if (world && physicsBody)
	{
		world->removeRigidBody(physicsBody);
		delete physicsBody->getMotionState();
		delete physicsBody->getCollisionShape();
		delete physicsBody;
		physicsBody = nullptr;
	}
}

void Player::drawDebugCapsule(bool drawRaycast)
{
	if (!showDebug || !physicsBody)
		return;

	Vector3 pos = getPosition();

	// Draw capsule body (cylinder)
	DrawCylinderWires(
		{pos.x, pos.y, pos.z},
		PLAYER_RADIUS,
		PLAYER_RADIUS,
		PLAYER_HEIGHT,
		DEBUG_CAPSULE_SEGMENTS,
		DEBUG_CAPSULE_COLOR);

	// Draw top hemisphere
	Vector3 topCenter = {pos.x, pos.y + PLAYER_HEIGHT / 2, pos.z};
	DrawSphereWires(topCenter, PLAYER_RADIUS, DEBUG_CAPSULE_SEGMENTS, DEBUG_CAPSULE_SEGMENTS, DEBUG_CAPSULE_COLOR);

	// Draw bottom hemisphere
	Vector3 bottomCenter = {pos.x, pos.y - PLAYER_HEIGHT / 2, pos.z};
	DrawSphereWires(bottomCenter, PLAYER_RADIUS, DEBUG_CAPSULE_SEGMENTS, DEBUG_CAPSULE_SEGMENTS, DEBUG_CAPSULE_COLOR);

	// Draw ground check raycast
	if (drawRaycast)
	{
		float rayLength = PLAYER_HEIGHT / 2 + 0.5f;
		Vector3 rayEnd = {pos.x, pos.y - rayLength, pos.z};
		DrawLine3D(pos, rayEnd, DEBUG_RAYCAST_COLOR);
	}
}