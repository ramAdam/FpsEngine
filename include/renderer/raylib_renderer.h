// include/renderer/raylib_renderer.h
#pragma once
#include "raylib.h"
#include "renderer_interface.h"

class RaylibRenderer : public RendererInterface {
private:
	Camera3D camera;
	int cameraMode = CAMERA_FIRST_PERSON; // Add camera mode
	float cameraSpeed = 0.1f;
	// Cache for uploaded meshes using a hash or ID
	std::unordered_map<size_t, Mesh> mesh_cache;
	size_t generate_mesh_id(const MeshData &mesh);
	size_t generate_robust_mesh_id(const MeshData &mesh);
	void update_camera_controls(); // New method for camera control

public:
	void init(int width, int height, const char *title) override;
	void begin_frame() override;
	void end_frame() override;
	void cleanup() override;
	void render_mesh(const MeshData &mesh) override;
	void set_camera_speed(float speed);
	void set_camera_mode(int mode); // New method to change camera mode
	void upload_mesh_to_gpu(const MeshData &mesh, size_t mesh_id);
};