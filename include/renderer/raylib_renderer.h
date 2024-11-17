// include/renderer/raylib_renderer.h
#pragma once

#include "raylib.h"
#include "renderer_interface.h"
#include <include/bsp/bsp.h>

class RaylibRenderer : public RendererInterface {
private:
	std::unique_ptr<BSPTree> bsp_tree;
	Camera3D camera;
	Vector3 player_start;
	int cameraMode = CAMERA_FIRST_PERSON; // Add camera mode
	float cameraSpeed = 0.5f;
	// Cache for uploaded meshes using a hash or ID
	std::unordered_map<size_t, Mesh> mesh_cache;
	size_t generate_mesh_id(const MeshData &mesh);
	size_t generate_robust_mesh_id(const MeshData &mesh);
	bool show_grid = true;
	bool show_wireframe = false;
	bool show_axes = true;
	float grid_spacing = 1.0f;
	int grid_slices = 10;
	bool mouse_locked = false;
	Model model; // Add Model member for OBJ
	bool model_loaded;
	void handle_camera_input(); // Add this method declaration

public:
	void init(int width, int height, const char *title) override;
	void begin_frame() override;
	void end_frame() override;
	void cleanup() override;
	void render_mesh(const MeshData &mesh) override;
	void set_camera_speed(float speed);
	void set_camera_mode(int mode); // New method to change camera mode
	void toggle_grid();
	void toggle_wireframe();
	void set_grid_spacing(float spacing);
	void draw_mesh_wireframe(const Mesh &mesh);
	void upload_mesh_to_gpu(const MeshData &mesh, size_t mesh_id);
	void toggle_mouse_lock();
	bool load_obj(const char *filename);
	void set_player_start(const Vector3 &position); // New method to set player start position
	bool check_collision(const Vector3 &position);
};