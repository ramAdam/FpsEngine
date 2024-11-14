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
	// Convert MeshData to raylib Mesh
	Mesh rlMesh = { 0 };
	rlMesh.vertexCount = mesh.vertices.size();
	rlMesh.triangleCount = mesh.indices.size() / 3;

	// Set vertex attributes
	rlMesh.vertices = (float *)mesh.vertices.data();
	rlMesh.normals = (float *)mesh.normals.data();
	rlMesh.texcoords = (float *)mesh.uvs.data();
	rlMesh.indices = (unsigned short *)mesh.indices.data();

	// Upload mesh to GPU
	UploadMesh(&rlMesh, false);

	// Draw mesh
	DrawMesh(rlMesh, LoadMaterialDefault(), MatrixIdentity());
}