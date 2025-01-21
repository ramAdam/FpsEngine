// include/renderer/raylib_renderer.h
#pragma once

#include "physics_manager.h"
#include "player.h"
#include "raylib.h"
#include "renderer_interface.h"
#include <include/bsp/bsp.h>

class RaylibRenderer : public RendererInterface
{
private:
	std::unique_ptr<BSPTree> bsp_tree;
	Model model;
	Camera3D camera;
	Player player;
	Vector3 player_start;
	int cameraMode = CAMERA_FIRST_PERSON;
	float cameraSpeed = 0.5f;
	bool show_grid = true;
	bool show_wireframe = false;
	bool show_axes = true;
	float grid_spacing = 1.0f;
	int grid_slices = 10;
	bool mouse_locked = false;
	bool model_loaded;
	void handle_camera_input(); // Add this method declaration
	float player_radius = 1.0f; // Collision sphere radius
	PhysicsManager physics;

	// Add this method declaration
	void draw_polygon(const Polygon &poly, Color color);
	void draw_polygons(const std::vector<Polygon> &polys, Color color);

public:
	void init(int width, int height, const char *title) override;
	void begin_frame() override;
	void end_frame() override;
	void cleanup() override;
	void render_mesh() override;
	void set_camera_speed(float speed) { cameraSpeed = speed; };
	void set_camera_mode(int mode) { cameraMode = mode; };
	void toggle_grid() { show_grid = !show_grid; }
	void toggle_wireframe() { show_wireframe = !show_wireframe; }
	void set_grid_spacing(float spacing) { grid_spacing = spacing; }

	void toggle_mouse_lock();
	bool load_obj(const char *filename);
	void set_player_start(const Vector3 &position); // New method to set player start position

	void set_player_radius(float radius) { player_radius = radius; }
};