#pragma once
#include "navigation_mesh.h"
#include <glm/glm.hpp>
#include <vector>

struct AStarNode {
	int triangle_idx;
	float g_cost;
	float f_cost;
	int came_from;

	// Add default constructor
	AStarNode() :
			triangle_idx(-1), g_cost(0), f_cost(0), came_from(-1) {}

	AStarNode(int idx, float g, float f, int prev) :
			triangle_idx(idx), g_cost(g), f_cost(f), came_from(prev) {}
};

class Pathfinder {
public:
	Pathfinder(const NavigationMesh *nav_mesh);
	std::vector<glm::vec3> find_path(const glm::vec3 &start, const glm::vec3 &end);

private:
	const NavigationMesh *nav_mesh;
	float heuristic(int triangle_idx, const glm::vec3 &target);
	std::vector<int> find_triangle_path(int start_tri, int end_tri);
	std::pair<int, int> find_shared_edge(const NavTriangle &t1, const NavTriangle &t2);
	bool is_better_path(const glm::vec3 &current, const glm::vec3 &left, const glm::vec3 &right);
};
