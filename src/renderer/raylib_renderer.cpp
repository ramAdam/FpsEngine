#include "raylib_renderer.h"
#include "raymath.h"
#include <cstring> // For std::memcpy
#include <functional> // For std::hash
#include <iostream>

void RaylibRenderer::init(int width, int height, const char *title) {
	InitWindow(width, height, title);
	SetTargetFPS(60);

	// Setup camera
	camera.position = (Vector3){ 10.0f, 5.0f, 10.0f };
	camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
	camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
	camera.fovy = 60.0f;
	camera.projection = CAMERA_PERSPECTIVE;

	// Initialize mouse locked state
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

void RaylibRenderer::begin_frame() {
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
		// if (show_wireframe) {
		// 	DrawModelWires(model, Vector3Zero(), 1.0f, WHITE);
		// } else {
		DrawModel(model, Vector3Zero(), 1.0f, WHITE);
		// }
	}
}

bool RaylibRenderer::load_obj(const char *filename) {
	if (!FileExists(filename)) {
		std::cerr << "Error: Could not find OBJ file: " << filename << std::endl;
		model_loaded = false;
		return false;
	}

	model = LoadModel(filename);

	// Check if model loaded successfully
	if (model.meshCount == 0) {
		std::cerr << "Error: Failed to load OBJ model: " << filename << std::endl;
		model_loaded = false;
		return false;
	}

	model_loaded = true;
	std::cout << "Successfully loaded model: " << filename << std::endl;
	return true;
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