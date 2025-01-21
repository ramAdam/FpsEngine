#pragma once
#include "pathfinding.h"
#include <glm/glm.hpp>

class AIController {
public:
	AIController(NavigationMesh *nav_mesh);
	void set_target(const glm::vec3 &target);
	void update(float delta_time);
	glm::vec3 get_current_position() const;
	void set_speed(float speed);

private:
	Pathfinder pathfinder;
	glm::vec3 current_position;
	glm::vec3 target_position;
	std::vector<glm::vec3> current_path;
	float movement_speed;
	size_t current_path_index;
};
