#include "raylib_renderer.h"
#include "raymath.h"
#include "camera_manager.h"
#include <cstring>	  // For std::memcpy
#include <functional> // For std::hash
#include <iostream>
#include "input_manager.h"
#include "debug_renderer.h"
#include "resource_manager.h"

RaylibRenderer::RaylibRenderer()
{
	cameraManager = std::make_unique<CameraManager>();
	resourceManager = std::make_unique<ResourceManager>(physics);
	debugRenderer = std::make_unique<DebugRenderer>();
}

RaylibRenderer::~RaylibRenderer()
{
	if (IsWindowReady())
	{ // Check if window is still valid
		player.cleanup();
		physics.cleanup();
		resourceManager->unloadAll();
		CloseWindow();
	}
}

void RaylibRenderer::init(int width, int height, const char *title)
{
	InitWindow(width, height, title);
	SetTargetFPS(60);

	// Initialize input and lock mouse by default for FPS controls
	auto &input = InputManager::getInstance();
	input.toggleMouseLock();

	cameraManager->init({0.0f, 20.0f, 10.0f});
	physics.init();
	player.init(physics.getDynamicsWorld(), cameraManager->getCamera().position);
}

void RaylibRenderer::handleInput()
{
	auto &input = InputManager::getInstance();
	input.update();

	// Camera mode switching
	if (input.isActionJustPressed(InputAction::TOGGLE_CAMERA_MODE))
	{
		cameraManager->toggleMode();
	}

	// Debug toggles
	if (input.isActionJustPressed(InputAction::TOGGLE_DEBUG))
	{
		debugRenderer->toggleGrid();
	}

	// Mouse lock toggle
	if (IsKeyPressed(KEY_ESCAPE))
	{
		input.toggleMouseLock();
	}

	// Player input is now handled within the Player class
	if (cameraManager->getCurrentMode() == CAMERA_FIRST_PERSON)
	{
		Vector2 mouseDelta = input.getMouseDelta();
		player.handleMouseInput(mouseDelta.x, mouseDelta.y);
	}
}

void RaylibRenderer::updateSimulation(float deltaTime)
{
	physics.update(deltaTime);
	player.update(deltaTime);
}

void RaylibRenderer::renderScene()
{
	BeginDrawing();
	ClearBackground(DARKGRAY);

	if (cameraManager->getCurrentMode() == CAMERA_FIRST_PERSON)
	{
		BeginMode3D(player.getCamera());
	}
	else
	{ //
		cameraManager->update();
		// 		BeginMode3D(cameraManager->getCamera());
		player.drawDebugCapsule();
	}

	render_mesh();

	EndMode3D();
	DrawFPS(10, 10);
	EndDrawing();
}

void RaylibRenderer::begin_frame()
{
	handleInput();
	updateSimulation(GetFrameTime());
	renderScene();
}

void RaylibRenderer::end_frame()
{
	// This function is now empty as its functionality has been moved to renderScene()
}

void RaylibRenderer::cleanup()
{
	// Empty function as cleanup is handled in destructor
}

bool RaylibRenderer::load_obj(const char *filename)
{
	return resourceManager->loadModel("main", filename);
}

void RaylibRenderer::render_mesh()
{
	Model *model = resourceManager->getModel("main");
	if (model)
	{
		Vector3 position = {0.0f, 0.0f, 0.0f};
		float scale = 0.5f;
		DrawModel(*model, position, scale, WHITE);
	}
}
