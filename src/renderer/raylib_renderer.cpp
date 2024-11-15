#include "raylib_renderer.h"
#include "raymath.h"

void RaylibRenderer::init(int width, int height, const char *title) {
	InitWindow(width, height, title);
	SetTargetFPS(60);

	camera.position = Vector3{ 10.0f, 10.0f, 10.0f };
	camera.target = Vector3{ 0.0f, 0.0f, 0.0f };
	camera.up = Vector3{ 0.0f, 1.0f, 0.0f };
	camera.fovy = 45.0f;
	camera.projection = CAMERA_PERSPECTIVE;
}

// Helper to generate mesh hash/ID
size_t RaylibRenderer::generate_mesh_id(const MeshData &mesh) {
	// Simple hash combining vertex count and indices
	return std::hash<size_t>{}(mesh.vertices.size()) ^
		   std::hash<size_t>{}(mesh.indices.size());
}

void RaylibRenderer::begin_frame() {
	BeginDrawing();
	ClearBackground(RAYWHITE);
	BeginMode3D(camera);
}

void RaylibRenderer::end_frame() {
	EndMode3D();
	EndDrawing();
}

void RaylibRenderer::cleanup() {
	CloseWindow();
}

void RaylibRenderer::render_mesh(const MeshData &mesh) {
	if (mesh.vertices.empty())
		return;

	size_t mesh_id = generate_mesh_id(mesh);

	// Check if mesh is already uploaded
	if (mesh_cache.find(mesh_id) == mesh_cache.end()) {
		// New mesh - upload to GPU
		Mesh rlMesh = { 0 };
		rlMesh.vertexCount = static_cast<int>(mesh.vertices.size());
		rlMesh.triangleCount = static_cast<int>(mesh.indices.size() / 3);

		if (!mesh.vertices.empty())
			rlMesh.vertices = (float *)mesh.vertices.data();
		if (!mesh.normals.empty())
			rlMesh.normals = (float *)mesh.normals.data();
		if (!mesh.uvs.empty())
			rlMesh.texcoords = (float *)mesh.uvs.data();

		UploadMesh(&rlMesh, false);
		mesh_cache[mesh_id] = rlMesh;
	}

	// Cache default material
	static Material defaultMat = LoadMaterialDefault();
	DrawMesh(mesh_cache[mesh_id], defaultMat, MatrixIdentity());
}