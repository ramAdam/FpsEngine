#include "ai_controller.h"

AIController::AIController(NavigationMesh *nav_mesh) :
		pathfinder(nav_mesh), movement_speed(5.0f), current_path_index(0) {}

void AIController::set_target(const glm::vec3 &target) {
	target_position = target;
	current_path = pathfinder.find_path(current_position, target_position);
	current_path_index = 0;
}

void AIController::update(float delta_time) {
	if (current_path_index >= current_path.size())
		return;

	glm::vec3 target = current_path[current_path_index];
	glm::vec3 direction = glm::normalize(target - current_position);

	current_position += direction * movement_speed * delta_time;

	if (glm::distance(current_position, target) < 0.1f) {
		current_path_index++;
	}
}

glm::vec3 AIController::get_current_position() const {
	return current_position;
}

void AIController::set_speed(float speed) {
	movement_speed = speed;
}
