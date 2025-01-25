#include "raylib_renderer.h"
#include "raymath.h"
#include "camera_manager.h"
#include <cstring>	  // For std::memcpy
#include <functional> // For std::hash
#include <iostream>
#include "input_manager.h"
#include "debug_renderer.h"
#include "resource_manager.h"
#include "pathfinding.h"
#include "navigation_mesh.h"

RaylibRenderer::RaylibRenderer()
{
	cameraManager = std::make_unique<CameraManager>();
	resourceManager = std::make_unique<ResourceManager>(physics);
	debugRenderer = std::make_unique<DebugRenderer>();
	navMesh = std::make_unique<NavigationMesh>();

	// Add some default test paths
	addPathTest({1, 1, 10}, {0, 1, 10});
	addPathTest({2, 1, 15}, {0, 1, 10});
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
		std::cout << "Toggling debug" << std::endl;
		debugRenderer->toggleGrid();
		debugRenderer->toggleNavMesh();
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

	if (IsKeyPressed(KEY_P))
	{ // Add path debug toggle
		std::cout << "Toggling path debug" << std::endl;
		togglePathDebug();
		toggleNavMeshDebug();
	}
}

void RaylibRenderer::update(float deltaTime)
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
	{
		cameraManager->update();
		BeginMode3D(cameraManager->getCamera());
		player.drawDebugCapsule();
	}

	render_mesh();

	if (debugRenderer->isNavMeshVisible() && navMesh)
	{
		debugRenderer->drawNavMesh(*navMesh);
		debugRenderer->drawNavMeshBounds(*navMesh);
	}
	drawDebugPaths();
	drawNavMeshDebug();

	EndMode3D();
	DrawFPS(10, 10);
	DrawText("Press c to toggle camera mode", 10, 30, 10, WHITE);
	EndDrawing();
}

void RaylibRenderer::processFrame()
{
	handleInput();
	update(GetFrameTime());
	renderScene();
}

bool RaylibRenderer::load_obj(const char *filename)
{
	bool model_loaded = resourceManager->loadModel("main", filename);
	if (model_loaded && navMesh)
	{
		const Model &model = *resourceManager->getModel("main");
		navMesh->buildFromMesh(model);
		updatePathTests();
	}

	return model_loaded;
}

void RaylibRenderer::render_mesh()
{
	Model *model = resourceManager->getModel("main");
	if (model)
	{
		Vector3 position = {0.0f, 0.0f, 0.0f};
		float scale = 1.0f;
		DrawModel(*model, position, scale, WHITE);
	}
}

void RaylibRenderer::addPathTest(const glm::vec3 &start, const glm::vec3 &end)
{
	pathTests.push_back({start, end, {}});
}

void RaylibRenderer::updatePathTests()
{
	if (!navMesh)
	{
		std::cout << "No nav mesh available" << std::endl;
		return;
	}

	Pathfinder pathfinder(navMesh.get());
	for (auto &test : pathTests)
	{
		std::cout << "Finding path from: "
				  << test.start.x << "," << test.start.y << "," << test.start.z
				  << " to: "
				  << test.end.x << "," << test.end.y << "," << test.end.z << std::endl;

		test.currentPath = pathfinder.find_path(test.start, test.end);
		std::cout << "Path size: " << test.currentPath.size() << std::endl;
	}
}

void RaylibRenderer::cleanup()
{
	// Empty function as cleanup is handled in destructor
}

void RaylibRenderer::drawDebugPaths()
{
	if (!showPathDebug)
		return;
	// std::cout << "PathTests size: " << pathTests.size() << std::endl;
	for (const auto &test : pathTests)
	{
		// Draw start and end points
		DrawSphere({test.start.x, test.start.y + 0.1f, test.start.z}, 0.3f, GREEN);
		DrawSphere({test.end.x, test.end.y + 0.1f, test.end.z}, 0.3f, RED);

		// Check if path exists before drawing
		if (test.currentPath.size() > 1)
		{
			// Draw path segments
			for (size_t i = 0; i < test.currentPath.size() - 1; i++)
			{
				Vector3 current = {
					test.currentPath[i].x,
					test.currentPath[i].y + 0.1f,
					test.currentPath[i].z};
				Vector3 next = {
					test.currentPath[i + 1].x,
					test.currentPath[i + 1].y + 0.1f,
					test.currentPath[i + 1].z};
				DrawLine3D(current, next, YELLOW);
			}
		}
	}
}

void RaylibRenderer::drawNavMeshDebug()
{
	if (!showNavMeshDebug || !navMesh)
		return;

	for (size_t i = 0; i < navMesh->getTriangleCount(); i++)
	{
		const auto &tri = navMesh->getTriangle(i);
		Color color = GRAY;

		if (i == 82 || i == 59)
		{
			color = RED;

			// Draw neighbors
			auto neighbors = navMesh->getTriangleNeighbors(i);
			glm::vec3 center = navMesh->getTriangleCenter(i);

			for (int neighbor : neighbors)
			{
				glm::vec3 nCenter = navMesh->getTriangleCenter(neighbor);
				DrawLine3D(
					{center.x, center.y + 0.1f, center.z},
					{nCenter.x, nCenter.y + 0.1f, nCenter.z},
					YELLOW);
			}
		}

		// Get vertex positions
		const auto &v1 = navMesh->getVertex(tri.v1());
		const auto &v2 = navMesh->getVertex(tri.v2());
		const auto &v3 = navMesh->getVertex(tri.v3());

		// Draw filled triangle
		DrawTriangle3D(
			{v1.x, v1.y, v1.z},
			{v2.x, v2.y, v2.z},
			{v3.x, v3.y, v3.z},
			color);

		// Draw wireframe using individual lines
		DrawLine3D({v1.x, v1.y, v1.z}, {v2.x, v2.y, v2.z}, BLACK);
		DrawLine3D({v2.x, v2.y, v2.z}, {v3.x, v3.y, v3.z}, BLACK);
		DrawLine3D({v3.x, v3.y, v3.z}, {v1.x, v1.y, v1.z}, BLACK);
	}
}