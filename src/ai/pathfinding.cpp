#include "pathfinding.h"
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <iostream>

Pathfinder::Pathfinder(const NavigationMesh *mesh) : nav_mesh(mesh) {}

std::vector<glm::vec3> Pathfinder::find_path(const glm::vec3 &start, const glm::vec3 &end)
{
	std::cout << "\n=== Starting Pathfinding ===\n";
	std::cout << "Start point: " << start.x << "," << start.y << "," << start.z << "\n";
	std::cout << "End point: " << end.x << "," << end.y << "," << end.z << "\n";

	// Get start and end triangles
	int startTri = nav_mesh->findNearestTriangle(start);
	int endTri = nav_mesh->findNearestTriangle(end);

	std::cout << "Start triangle: " << startTri << "\n";
	std::cout << "End triangle: " << endTri << "\n";

	// Validate triangles
	if (startTri < 0 || endTri < 0)
	{
		std::cout << "Failed to find valid triangles for path endpoints\n";
		return std::vector<glm::vec3>();
	}

	// A* pathfinding
	std::vector<int> trianglePath = find_triangle_path(startTri, endTri);
	std::cout << "Triangle path size: " << trianglePath.size() << "\n";

	if (trianglePath.empty())
	{
		return std::vector<glm::vec3>();
	}

	// Convert triangle path to waypoints
	std::vector<glm::vec3> path = generate_waypoints(trianglePath, start, end);
	std::cout << "Final path size: " << path.size() << "\n";

	// Debug triangle path
	std::cout << "Triangle path: ";
	for (int tri : trianglePath)
	{
		std::cout << tri << " ";
	}
	std::cout << "\n";

	return path;
}

std::pair<int, int> Pathfinder::find_shared_edge(const NavTriangle &t1, const NavTriangle &t2)
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			if ((t1.vertices[i] == t2.vertices[j] &&
				 t1.vertices[(i + 1) % 3] == t2.vertices[(j + 2) % 3]) ||
				(t1.vertices[i] == t2.vertices[(j + 2) % 3] &&
				 t1.vertices[(i + 1) % 3] == t2.vertices[j]))
			{
				return {t1.vertices[i], t1.vertices[(i + 1) % 3]};
			}
		}
	}
	return {-1, -1};
}

bool Pathfinder::is_better_path(const glm::vec3 &current,
								const glm::vec3 &left,
								const glm::vec3 &right)
{
	// Simple angle check for better path
	glm::vec3 cur_dir = glm::normalize(current - left);
	glm::vec3 new_dir = glm::normalize(right - left);
	return glm::dot(cur_dir, new_dir) < 0.7f;
}

std::vector<int> Pathfinder::find_triangle_path(int startTri, int endTri)
{
	std::cout << "\nDebug triangle path finding:\n";
	std::cout << "Start triangle: " << startTri << ", End triangle: " << endTri << "\n";

	// Initialize data structures for A*
	std::vector<bool> closedSet(nav_mesh->getTriangleCount(), false);
	std::unordered_map<int, int> cameFrom;
	std::unordered_map<int, float> gScore;
	std::priority_queue<std::pair<float, int>> openSet;

	// Set initial values
	gScore[startTri] = 0;
	openSet.push({0, startTri});

	while (!openSet.empty())
	{
		int current = openSet.top().second;
		openSet.pop();

		if (current == endTri)
		{
			// Path found - reconstruct
			std::vector<int> path;
			while (current != startTri)
			{
				path.push_back(current);
				current = cameFrom[current];
			}
			path.push_back(startTri);
			std::reverse(path.begin(), path.end());
			return path;
		}

		// Debug current triangle's neighbors
		auto neighbors = nav_mesh->getTriangleNeighbors(current);
		std::cout << "Triangle " << current << " neighbors: ";
		for (int n : neighbors)
		{
			std::cout << n << " ";
		}
		std::cout << "\n";

		for (int neighbor : neighbors)
		{
			if (closedSet[neighbor])
				continue;

			float tentative_gScore = gScore[current] + 1.0f;
			if (!gScore.count(neighbor) || tentative_gScore < gScore[neighbor])
			{
				cameFrom[neighbor] = current;
				gScore[neighbor] = tentative_gScore;
				float fScore = tentative_gScore + heuristic(neighbor, nav_mesh->getTriangleCenter(endTri));
				openSet.push({-fScore, neighbor});
			}
		}
		closedSet[current] = true;
	}

	std::cout << "No path found between triangles " << startTri << " and " << endTri << "\n";
	return std::vector<int>();
}

float Pathfinder::heuristic(int triangle_idx, const glm::vec3 &target)
{
	// Get triangle center
	const NavTriangle &tri = nav_mesh->getTriangle(triangle_idx);
	glm::vec3 center = (nav_mesh->getVertex(tri.vertices[0]) +
						nav_mesh->getVertex(tri.vertices[1]) +
						nav_mesh->getVertex(tri.vertices[2])) /
					   3.0f;

	return glm::distance(center, target);
}

std::vector<glm::vec3> Pathfinder::generate_waypoints(
	const std::vector<int> &trianglePath,
	const glm::vec3 &start,
	const glm::vec3 &end)
{
	std::vector<glm::vec3> waypoints;

	// Add start point
	waypoints.push_back(start);

	// Generate midpoints for each triangle transition
	for (size_t i = 0; i < trianglePath.size() - 1; ++i)
	{
		int currentTri = trianglePath[i];
		int nextTri = trianglePath[i + 1];

		// Find shared edge midpoint
		const auto &tri1 = nav_mesh->getTriangle(currentTri);
		const auto &tri2 = nav_mesh->getTriangle(nextTri);

		// Find shared vertices
		std::vector<int> sharedVerts;
		for (int v1 : tri1.vertices)
		{
			for (int v2 : tri2.vertices)
			{
				if (v1 == v2)
				{
					sharedVerts.push_back(v1);
				}
			}
		}

		if (sharedVerts.size() == 2)
		{
			// Calculate midpoint of shared edge
			const auto &v1 = nav_mesh->getVertex(sharedVerts[0]);
			const auto &v2 = nav_mesh->getVertex(sharedVerts[1]);
			glm::vec3 midpoint = (v1 + v2) * 0.5f;
			waypoints.push_back(midpoint);
		}
	}

	// Add end point
	waypoints.push_back(end);

	return waypoints;
}
