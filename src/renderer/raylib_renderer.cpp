#include "raylib_renderer.h"
#include "raymath.h"
#include <cstring> // For std::memcpy
#include <functional> // For std::hash
#include <iostream>

void RaylibRenderer::init(int width, int height, const char *title) {
	InitWindow(width, height, title);
	SetTargetFPS(60);

	// Initialize default camera
	camera.position = (Vector3){ 0.0f, 2.0f, 10.0f };
	set_player_start(camera.position);
	camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
	camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
	camera.fovy = 60.0f;
	camera.projection = CAMERA_PERSPECTIVE;
	DisableCursor();
	mouse_locked = true;
}

// Helper to generate mesh hash/ID
size_t RaylibRenderer::generate_mesh_id(const MeshData &mesh) {
	// Simple hash combining vertex count and indices
	return std::hash<size_t>{}(mesh.vertices.size()) ^
		   std::hash<size_t>{}(mesh.indices.size());
}

size_t RaylibRenderer::generate_robust_mesh_id(const MeshData &mesh) {
	// Create a hash combining vertex and index data
	size_t hash = 0;

	// Hash vertex positions
	for (const auto &vertex : mesh.vertices) {
		hash ^= std::hash<float>{}(vertex.x) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		hash ^= std::hash<float>{}(vertex.y) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
		hash ^= std::hash<float>{}(vertex.z) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
	}

	// Hash indices
	for (const auto &index : mesh.indices) {
		hash ^= std::hash<int>{}(index) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
	}

	return hash;
}

void RaylibRenderer::handle_camera_input() {
	Vector3 newPos = camera.position;

	if (IsKeyPressed(KEY_ONE)) {
		cameraMode = CAMERA_FREE;
		std::cout << "Camera Mode: FREE" << std::endl;
	} else if (IsKeyPressed(KEY_TWO)) {
		cameraMode = CAMERA_FIRST_PERSON;
		camera.position = player_start;
		std::cout << "Camera Mode: FIRST PERSON" << std::endl;
	}
}

void RaylibRenderer::begin_frame() {
	handle_camera_input();
	UpdateCamera(&camera, cameraMode); // Use stored camera mode
	BeginDrawing();
	ClearBackground(DARKGRAY);
	BeginMode3D(camera);

	// Draw debug grid
	if (show_grid) {
		DrawGrid(grid_slices, grid_spacing);
		DrawLine3D({ 0, 0, 0 }, { 5, 0, 0 }, RED); // X axis
		DrawLine3D({ 0, 0, 0 }, { 0, 5, 0 }, GREEN); // Y axis
		DrawLine3D({ 0, 0, 0 }, { 0, 0, 5 }, BLUE); // Z axis
	}
}

void RaylibRenderer::end_frame() {
	EndMode3D();
	EndDrawing();
}

void RaylibRenderer::cleanup() {
	if (model_loaded) {
		UnloadModel(model);
	}
	CloseWindow();
}

void RaylibRenderer::upload_mesh_to_gpu(const MeshData &mesh, size_t mesh_id) {
	Mesh rlMesh = { 0 };

	// Set counts
	rlMesh.vertexCount = static_cast<int>(mesh.vertices.size());
	rlMesh.triangleCount = static_cast<int>(mesh.indices.size() / 3);

	// Allocate and copy vertex data
	rlMesh.vertices = static_cast<float *>(MemAlloc(mesh.vertices.size() * 3 * sizeof(float)));
	std::memcpy(rlMesh.vertices, mesh.vertices.data(), mesh.vertices.size() * 3 * sizeof(float));

	// Copy normals if present
	if (!mesh.normals.empty()) {
		rlMesh.normals = static_cast<float *>(MemAlloc(mesh.normals.size() * 3 * sizeof(float)));
		std::memcpy(rlMesh.normals, mesh.normals.data(), mesh.normals.size() * 3 * sizeof(float));
	}

	// Copy UVs if present
	if (!mesh.uvs.empty()) {
		rlMesh.texcoords = static_cast<float *>(MemAlloc(mesh.uvs.size() * 2 * sizeof(float)));
		std::memcpy(rlMesh.texcoords, mesh.uvs.data(), mesh.uvs.size() * 2 * sizeof(float));
	}

	// Copy indices
	rlMesh.indices = static_cast<unsigned short *>(MemAlloc(mesh.indices.size() * sizeof(unsigned short)));
	std::memcpy(rlMesh.indices, mesh.indices.data(), mesh.indices.size() * sizeof(unsigned short));

	// Upload to GPU
	UploadMesh(&rlMesh, false);

	// Store in cache
	mesh_cache[mesh_id] = rlMesh;
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
		float scale = 0.5f; // Scale down the model

		DrawModel(model, position, scale, WHITE);

		if (show_wireframe) {
			DrawModelWires(model, position, scale, RED);
		}
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
		bsp_tree = std::make_unique<BSPTree>();
		bsp_tree->build(model.meshes[0]);

		model_loaded = true;
		return true;

	} catch (const std::exception &e) {
		std::cerr << "Error loading model: " << e.what() << std::endl;
		model_loaded = false;
		return false;
	}
}

// Optional: Add camera control methods
void RaylibRenderer::set_camera_speed(float speed) {
	cameraSpeed = speed;
}

void RaylibRenderer::set_camera_mode(int mode) {
	cameraMode = mode;
}

// Add control methods
void RaylibRenderer::toggle_grid() {
	show_grid = !show_grid;
}
void RaylibRenderer::toggle_wireframe() {
	show_wireframe = !show_wireframe;
}
void RaylibRenderer::set_grid_spacing(float spacing) {
	grid_spacing = spacing;
}

void RaylibRenderer::draw_mesh_wireframe(const Mesh &mesh) {
	if (!mesh.vertices || !mesh.indices)
		return;

	const float *vertices = static_cast<const float *>(mesh.vertices);
	const unsigned short *indices = static_cast<const unsigned short *>(mesh.indices);

	// Debug output for triangles
	for (int i = 0; i < mesh.triangleCount * 3; i += 3) {
		Vector3 v1 = {
			vertices[indices[i] * 3],
			vertices[indices[i] * 3 + 1],
			vertices[indices[i] * 3 + 2]
		};
		Vector3 v2 = {
			vertices[indices[i + 1] * 3],
			vertices[indices[i + 1] * 3 + 1],
			vertices[indices[i + 1] * 3 + 2]
		};
		Vector3 v3 = {
			vertices[indices[i + 2] * 3],
			vertices[indices[i + 2] * 3 + 1],
			vertices[indices[i + 2] * 3 + 2]
		};

		// Print triangle vertices for debugging
		TraceLog(LOG_DEBUG, "Triangle %d: (%f,%f,%f) (%f,%f,%f) (%f,%f,%f)",
				i / 3, v1.x, v1.y, v1.z, v2.x, v2.y, v2.z, v3.x, v3.y, v3.z);

		// Draw all edges in WHITE for better visibility
		DrawLine3D(v1, v2, WHITE);
		DrawLine3D(v2, v3, WHITE);
		DrawLine3D(v3, v1, WHITE);
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

void RaylibRenderer::set_player_start(const Vector3 &position) {
	player_start = position;
	camera.position = player_start;
}

bool RaylibRenderer::check_collision(const Vector3 &position) {
	return bsp_tree->check_collision(position, player_radius);
}

Vector3 RaylibRenderer::try_move(const Vector3 &current, const Vector3 &target) {
	// Check if new position would collide
	if (bsp_tree && bsp_tree->check_collision(target, player_radius)) {
		return current; // Keep old position if collision detected
	}
	return target; // Allow movement if no collision
}