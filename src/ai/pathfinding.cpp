#include "pathfinding.h"
#include <algorithm>
#include <queue>
#include <unordered_map>

Pathfinder::Pathfinder(const NavigationMesh *mesh) :
		nav_mesh(mesh) {}

std::vector<glm::vec3> Pathfinder::find_path(const glm::vec3 &start, const glm::vec3 &end) {
	int start_tri = nav_mesh->find_nearest_triangle(start);
	int end_tri = nav_mesh->find_nearest_triangle(end);

	std::vector<int> triangle_path = find_triangle_path(start_tri, end_tri);
	std::vector<glm::vec3> path;

	if (triangle_path.empty())
		return path;

	// Funnel algorithm implementation
	path.push_back(start);

	// For each triangle in the path, create a funnel
	for (size_t i = 0; i < triangle_path.size() - 1; i++) {
		const NavTriangle &current = nav_mesh->get_triangle(triangle_path[i]);
		const NavTriangle &next = nav_mesh->get_triangle(triangle_path[i + 1]);

		// Find shared edge
		std::pair<int, int> portal = find_shared_edge(current, next);

		// Add portal vertices to path if they create better angles
		glm::vec3 portal_left = nav_mesh->get_vertex(portal.first);
		glm::vec3 portal_right = nav_mesh->get_vertex(portal.second);

		if (is_better_path(path.back(), portal_left, portal_right)) {
			path.push_back(portal_left);
		}
	}

	path.push_back(end);
	return path;
}

std::pair<int, int> Pathfinder::find_shared_edge(const NavTriangle &t1, const NavTriangle &t2) {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			if ((t1.vertices[i] == t2.vertices[j] &&
						t1.vertices[(i + 1) % 3] == t2.vertices[(j + 2) % 3]) ||
					(t1.vertices[i] == t2.vertices[(j + 2) % 3] &&
							t1.vertices[(i + 1) % 3] == t2.vertices[j])) {
				return { t1.vertices[i], t1.vertices[(i + 1) % 3] };
			}
		}
	}
	return { -1, -1 };
}

bool Pathfinder::is_better_path(const glm::vec3 &current,
		const glm::vec3 &left,
		const glm::vec3 &right) {
	// Simple angle check for better path
	glm::vec3 cur_dir = glm::normalize(current - left);
	glm::vec3 new_dir = glm::normalize(right - left);
	return glm::dot(cur_dir, new_dir) < 0.7f;
}

std::vector<int> Pathfinder::find_triangle_path(int start_tri, int end_tri) {
	if (start_tri == -1 || end_tri == -1)
		return std::vector<int>();

	std::unordered_map<int, AStarNode> nodes;
	std::priority_queue<std::pair<float, int>,
			std::vector<std::pair<float, int>>,
			std::greater<std::pair<float, int>>>
			open_set;

	// Initialize start node
	nodes.emplace(
			start_tri,
			AStarNode(start_tri, 0.0f, heuristic(start_tri, nav_mesh->get_vertex(end_tri)), -1));
	open_set.push({ nodes[start_tri].f_cost, start_tri });

	while (!open_set.empty()) {
		int current = open_set.top().second;
		open_set.pop();

		if (current == end_tri) {
			// Reconstruct path
			std::vector<int> path;
			while (current != -1) {
				path.push_back(current);
				current = nodes[current].came_from;
			}
			std::reverse(path.begin(), path.end());
			return path;
		}

		const NavTriangle &tri = nav_mesh->get_triangle(current);
		for (int i = 0; i < 3; i++) {
			int neighbor = tri.neighbors[i];
			if (neighbor == -1)
				continue;

			float tentative_g = nodes[current].g_cost + 1.0f; // Using 1.0 as edge cost

			if (nodes.find(neighbor) == nodes.end() ||
					tentative_g < nodes[neighbor].g_cost) {
				float f_cost = tentative_g + heuristic(neighbor, nav_mesh->get_vertex(end_tri));
				nodes[neighbor] = AStarNode(neighbor, tentative_g, f_cost, current);
				open_set.push({ f_cost, neighbor });
			}
		}
	}

	return std::vector<int>();
}

float Pathfinder::heuristic(int triangle_idx, const glm::vec3 &target) {
	// Get triangle center
	const NavTriangle &tri = nav_mesh->get_triangle(triangle_idx);
	glm::vec3 center = (nav_mesh->get_vertex(tri.vertices[0]) +
							   nav_mesh->get_vertex(tri.vertices[1]) +
							   nav_mesh->get_vertex(tri.vertices[2])) /
					   3.0f;

	return glm::distance(center, target);
}
