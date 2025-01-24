#include "raylib_renderer.h"
#include <iostream>
#include <memory>

int main(int argc, char *argv[])
{
	auto renderer = std::make_unique<RaylibRenderer>();
	renderer->init(800, 600, "FPS Engine");
	// renderer->set_camera_speed(0.5f);
	// renderer->toggle_wireframe();

	std::string mapPath = "../maps/block.obj";

	if (renderer->load_obj(mapPath.c_str()))
	{
		std::cout << "Loaded model" << std::endl;
	}

	while (!WindowShouldClose())
	{
		renderer->processFrame();

		// renderer->render_mesh();
	}

	return 0;
}
