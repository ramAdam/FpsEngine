// include/renderer/raylib_renderer.h
#pragma once

#include "physics_manager.h"
#include "player.h"
#include "raylib.h"
#include "renderer_interface.h"
#include <include/bsp/bsp.h>

// Forward declarations
class CameraManager;
class ResourceManager;
class DebugRenderer; // Removed InputManager forward declaration

class RaylibRenderer : public RendererInterface
{
private:
	std::unique_ptr<CameraManager> cameraManager;
	std::unique_ptr<ResourceManager> resourceManager;
	std::unique_ptr<DebugRenderer> debugRenderer;

	PhysicsManager physics;
	Player player;

	void handleInput();
	void updateSimulation(float deltaTime);
	void renderScene();

public:
	RaylibRenderer();
	virtual ~RaylibRenderer() override; // Make destructor virtual and override

	// Core functionality
	void init(int width, int height, const char *title) override;
	void begin_frame() override;
	void end_frame() override;
	void cleanup() override;
	void render_mesh() override;
	bool load_obj(const char *filename);

	// Accessors
	CameraManager &getCameraManager() { return *cameraManager; }
	ResourceManager &getResourceManager() { return *resourceManager; }
	DebugRenderer &getDebugRenderer() { return *debugRenderer; }
};