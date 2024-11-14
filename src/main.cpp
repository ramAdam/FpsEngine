#include "raylib_renderer.h"
#include <iostream>
#include <memory>

int main(int argc, char *argv[]) {
	auto renderer = std::make_unique<RaylibRenderer>();
	renderer->init(800, 600, "Qodot Raylib Renderer");

	Qodot qodot;
	qodot.load_map("../maps/45_degree.map");

	while (!WindowShouldClose()) {
		renderer->begin_frame();

		auto meshes = qodot.fetch_surfaces(1.0);
		for (const auto &mesh : meshes) {
			renderer->render_mesh(mesh);
		}

		renderer->end_frame();
	}

	renderer->cleanup();
	return 0;
}