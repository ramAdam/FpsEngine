#include "raylib_renderer.h"
#include "raymath.h"
#include <cstring> // For std::memcpy
#include <functional> // For std::hash

void RaylibRenderer::init(int width, int height, const char *title) {
	InitWindow(width, height, title);
	SetTargetFPS(60);

	// Setup camera
	camera.position = (Vector3){ 10.0f, 5.0f, 10.0f };
	camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };
	camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
	camera.fovy = 60.0f;
	camera.projection = CAMERA_PERSPECTIVE;
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
}

void RaylibRenderer::end_frame() {
	EndMode3D();
	EndDrawing();
}

void RaylibRenderer::cleanup() {
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
	// Validate input
	if (mesh.vertices.empty() || mesh.indices.empty()) {
		TraceLog(LOG_WARNING, "Attempted to render empty mesh");
		return;
	}

	// Generate more robust mesh ID
	size_t mesh_id = generate_robust_mesh_id(mesh);

	// Upload if not cached
	if (auto it = mesh_cache.find(mesh_id); it == mesh_cache.end()) {
		upload_mesh_to_gpu(mesh, mesh_id);
	}

	// Render with cached material
	static const Material defaultMat = LoadMaterialDefault();
	DrawMesh(mesh_cache.at(mesh_id), defaultMat, MatrixIdentity());
}

// Optional: Add camera control methods
void RaylibRenderer::set_camera_speed(float speed) {
	cameraSpeed = speed;
	// SetCameraMoveControls(
	// 		KEY_W, KEY_S, KEY_D, KEY_A, KEY_E, KEY_Q);
}

void RaylibRenderer::set_camera_mode(int mode) {
	cameraMode = mode;
}