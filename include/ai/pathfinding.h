#pragma once
#include "navigation_mesh.h"
#include <glm/glm.hpp>
#include <vector>

struct AStarNode
{
	int triangleIndex;
	float gScore;
	float fScore;
	int cameFrom;

	// Add default constructor
	AStarNode() : triangleIndex(-1), gScore(0), fScore(0), cameFrom(-1) {}
	AStarNode(int index) : triangleIndex(index), gScore(std::numeric_limits<float>::infinity()), fScore(std::numeric_limits<float>::infinity()), cameFrom(-1) {}

	AStarNode(int idx, float g, float f, int prev) : triangleIndex(idx), gScore(g), fScore(f), cameFrom(prev) {}
};

class Pathfinder
{
public:
	Pathfinder(const NavigationMesh *nav_mesh);
	std::vector<glm::vec3> find_path(const glm::vec3 &start, const glm::vec3 &end);

private:
	const NavigationMesh *nav_mesh;
	float heuristic(int triangle_idx, const glm::vec3 &target);
	std::vector<int> find_triangle_path(int start_tri, int end_tri);
	std::pair<int, int> find_shared_edge(const NavTriangle &t1, const NavTriangle &t2);
	bool is_better_path(const glm::vec3 &current, const glm::vec3 &left, const glm::vec3 &right);

	std::vector<glm::vec3> generate_waypoints(
		const std::vector<int> &trianglePath,
		const glm::vec3 &start,
		const glm::vec3 &end);
};
