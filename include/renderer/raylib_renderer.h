// include/renderer/raylib_renderer.h
#pragma once
#include "raylib.h"
#include "renderer_interface.h"

class RaylibRenderer : public RendererInterface {
private:
	Camera3D camera;
	// Cache for uploaded meshes using a hash or ID
	std::unordered_map<size_t, Mesh> mesh_cache;
	size_t generate_mesh_id(const MeshData &mesh);

public:
	void init(int width, int height, const char *title) override;
	void begin_frame() override;
	void end_frame() override;
	void cleanup() override;
	void render_mesh(const MeshData &mesh) override;
};