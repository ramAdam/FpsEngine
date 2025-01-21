#include "raylib_renderer.h"
#include <map_loader.h>
#include <iostream>
#include <memory>

// After your existing code in main.cpp
void printMeshData(const MeshData &mesh, int meshIndex) {
	std::cout << "\n=== Mesh " << meshIndex << " Details ===" << std::endl;
	std::cout << "Vertices count: " << mesh.vertices.size() << std::endl;
	std::cout << "Normals count: " << mesh.normals.size() << std::endl;
	std::cout << "Tangents count: " << mesh.tangents.size() << std::endl;
	std::cout << "UVs count: " << mesh.uvs.size() << std::endl;
	std::cout << "Indices count: " << mesh.indices.size() << std::endl;

	// Optional: Print first few elements of each vector
	if (!mesh.vertices.empty()) {
		std::cout << "First vertex: ("
				  << mesh.vertices[0].x << ", "
				  << mesh.vertices[0].y << ", "
				  << mesh.vertices[0].z << ")" << std::endl;
	}
}

int main(int argc, char *argv[]) {
	auto renderer = std::make_unique<RaylibRenderer>();
	renderer->init(800, 600, "FPS Engine");
	renderer->set_camera_speed(0.5f);
	// renderer->toggle_wireframe();

	std::string mapPath = "../maps/block.obj";

	if (renderer->load_obj(mapPath.c_str())) {
		std::cout << "Loaded model" << std::endl;
	}

	while (!WindowShouldClose()) {
		renderer->begin_frame();

		renderer->render_mesh({});

		renderer->end_frame();
	}

	renderer->cleanup();
	return 0;
}
