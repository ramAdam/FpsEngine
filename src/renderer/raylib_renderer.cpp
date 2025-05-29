#include "raylib_renderer.h"
#include "raymath.h"
#include "camera_manager.h"
#include <cstring>
#include <functional>
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
    // navMesh = std::make_unique<NavigationMesh>();

    // Remove hardcoded test paths and do this elsewhere
}

RaylibRenderer::~RaylibRenderer()
{
    if (IsWindowReady())
    {
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
        debugRenderer->toggleCollisionShapes(); // Add this line
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

    // Debug visualizations
    if (debugRenderer->isNavMeshVisible() && navMesh)
    {
        debugRenderer->drawNavMesh(*navMesh);
    }

    // Add collision shape visualization
    debugRenderer->drawCollisionShapes(physics.getDynamicsWorld());

    EndMode3D();
    DrawFPS(10, 10);
    DrawText("Press c to toggle camera mode", 10, 30, 10, WHITE);

    // Add debug toggle information
    if (debugRenderer->isNavMeshVisible() || debugRenderer->isCollisionDebugVisible())
    {
        DrawText("Debug visualization ON", 10, 50, 10, GREEN);
    }

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

void RaylibRenderer::cleanup()
{
    // Empty function as cleanup is handled in destructor
}