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
	renderer->init(800, 600, "Raylib Renderer for Trench Broom Maps");

	std::string mapPath = "../maps/45_degree.map";

	MapLoader mapLoader;
	if (!mapLoader.load(mapPath)) {
		std::cerr << "Failed to load map" << std::endl;
		return 1;
	}

	auto meshes = mapLoader.getMeshes();

	while (!WindowShouldClose()) {
		renderer->begin_frame();

		for (const auto &mesh : meshes) {
			renderer->render_mesh(mesh);
		}

		renderer->end_frame();
	}

	renderer->cleanup();
	return 0;
}
