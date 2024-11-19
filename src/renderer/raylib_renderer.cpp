#include "raylib_renderer.h"
#include "raymath.h"
#include <cstring> // For std::memcpy
#include <functional> // For std::hash
#include <iostream>

void RaylibRenderer::init(int width, int height, const char *title) {
	InitWindow(width, height, title);
	SetTargetFPS(60);

	// Initialize default camera
	camera.position = (Vector3){ 0.0f, 20.0f, 10.0f };
	set_player_start(camera.position);
	camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
	camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
	camera.fovy = 60.0f;
	camera.projection = CAMERA_PERSPECTIVE;
	DisableCursor();
	mouse_locked = true;

	physics.init();
	player.init(physics.getDynamicsWorld(), camera.position);
	player.attachCamera(&camera);
}

void RaylibRenderer::handle_camera_input() {
	if (IsKeyPressed(KEY_ONE)) {
		cameraMode = CAMERA_FREE;
		std::cout << "Camera Mode: FREE" << std::endl;
	} else if (IsKeyPressed(KEY_TWO)) {
		cameraMode = CAMERA_FIRST_PERSON;
		player.setPosition(player_start);
		std::cout << "Camera Mode: FIRST PERSON" << std::endl;
	}
}

void RaylibRenderer::begin_frame() {
	handle_camera_input();

	// Update physics
	physics.update(GetFrameTime());
	player.update(GetFrameTime());

	UpdateCamera(&camera, cameraMode);
	BeginDrawing();
	ClearBackground(DARKGRAY);
	BeginMode3D(camera);

	// Draw debug grid
	// if (show_grid) {
	// 	DrawGrid(grid_slices, grid_spacing);
	// 	DrawLine3D({ 0, 0, 0 }, { 5, 0, 0 }, RED); // X axis
	// 	DrawLine3D({ 0, 0, 0 }, { 0, 5, 0 }, GREEN); // Y axis
	// 	DrawLine3D({ 0, 0, 0 }, { 0, 0, 5 }, BLUE); // Z axis
	// }
}

void RaylibRenderer::end_frame() {
	EndMode3D();
	EndDrawing();
}

void RaylibRenderer::cleanup() {
	player.cleanup();
	physics.cleanup();
	if (model_loaded) {
		UnloadModel(model);
	}
	CloseWindow();
}

void RaylibRenderer::render_mesh(const MeshData &mesh) {
	if (model_loaded) {
		// Draw reference grid and axes
		// DrawGrid(10, 1.0f);
		DrawLine3D({ 0, 0, 0 }, { 5, 0, 0 }, RED); // X
		DrawLine3D({ 0, 0, 0 }, { 0, 5, 0 }, GREEN); // Y
		DrawLine3D({ 0, 0, 0 }, { 0, 0, 5 }, BLUE); // Z

		// Adjust scale for better visibility
		Vector3 position = { 0.0f, 0.0f, 0.0f };
		// float scale = 0.5f; // Scale down the model
		float scale = 1.0f;

		// draw_bsp_polygons(*bsp_tree, 10);
		// DrawModel(model, position, scale, WHITE);
		DrawModelWires(model, position, scale, MAROON);
		// draw_polygons(bsp_tree->polygons, GREEN);
	}
}

bool RaylibRenderer::load_obj(const char *filename) {
	if (!FileExists(filename)) {
		std::cerr << "Error: Could not find OBJ file: " << filename << std::endl;
		return false;
	}

	try {
		model = LoadModel(filename);

		// Build BSP tree with validated mesh
		// bsp_tree = std::make_unique<BSPTree>();
		// bsp_tree->build(model.meshes[0]);

		model_loaded = true;
		return true;

	} catch (const std::exception &e) {
		std::cerr << "Error loading model: " << e.what() << std::endl;
		model_loaded = false;
		return false;
	}
}

void RaylibRenderer::set_player_start(const Vector3 &position) {
	player_start = position;
	camera.position = player_start;
}

void RaylibRenderer::draw_polygon(const Polygon &poly, Color color) {
	for (size_t i = 0; i < poly.vertices.size(); ++i) {
		const auto &v1 = poly.vertices[i];
		const auto &v2 = poly.vertices[(i + 1) % poly.vertices.size()];
		DrawLine3D(v1, v2, color);
	}
}

void RaylibRenderer::draw_polygons(const std::vector<Polygon> &polygons, Color color) {
	for (const auto &poly : polygons) {
		draw_polygon(poly, color);
	}
}

void RaylibRenderer::toggle_mouse_lock() {
	mouse_locked = !mouse_locked;
	if (mouse_locked) {
		DisableCursor();
	} else {
		EnableCursor();
	}
}