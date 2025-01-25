#pragma once
#include "raylib.h" // Add RayLib include
#include <glm/glm.hpp>
#include <vector>
#include <array> // Add array include

struct NavTriangle
{
	int vertices[3];
	int neighbors[3]; // Adjacent triangles

	const int v1() const { return vertices[0]; }
	const int v2() const { return vertices[1]; }
	const int v3() const { return vertices[2]; }
};

class NavigationMesh
{
public:
	NavigationMesh();
	void buildFromMesh(const Model &model, float max_slope_angle = 45.0f);
	int findNearestTriangle(const glm::vec3 &point) const;
	const NavTriangle &getTriangle(int index) const { return triangles[index]; }
	const glm::vec3 &getVertex(int index) const { return vertices[index]; }

	const std::vector<NavTriangle> &getTriangles() const { return triangles; }
	const std::vector<glm::vec3> &getVertices() const { return vertices; }

	size_t getTriangleCount() const { return triangles.size(); }

	glm::vec3 getTriangleCenter(int triangleIndex) const
	{
		const auto &tri = getTriangle(triangleIndex);
		return (vertices[tri.v1()] + vertices[tri.v2()] + vertices[tri.v3()]) / 3.0f;
	}

	std::vector<int> getTriangleNeighbors(int triangleIndex) const
	{
		std::vector<int> neighbors;

		if (triangleIndex < 0 || triangleIndex >= triangles.size())
		{
			return neighbors;
		}

		// Get edges of current triangle
		const auto &tri = triangles[triangleIndex];
		std::array<std::pair<int, int>, 3> edges = {
			std::make_pair(tri.vertices[0], tri.vertices[1]),
			std::make_pair(tri.vertices[1], tri.vertices[2]),
			std::make_pair(tri.vertices[2], tri.vertices[0])};

		// Check all other triangles
		for (size_t i = 0; i < triangles.size(); i++)
		{
			if (i == triangleIndex)
				continue;

			const auto &other = triangles[i];
			std::array<std::pair<int, int>, 3> otherEdges = {
				std::make_pair(other.vertices[0], other.vertices[1]),
				std::make_pair(other.vertices[1], other.vertices[2]),
				std::make_pair(other.vertices[2], other.vertices[0])};

			// Check for shared edge
			for (const auto &edge : edges)
			{
				for (const auto &otherEdge : otherEdges)
				{
					if ((edge.first == otherEdge.first && edge.second == otherEdge.second) ||
						(edge.first == otherEdge.second && edge.second == otherEdge.first))
					{
						neighbors.push_back(i);
						goto nextTriangle; // Found shared edge, check next triangle
					}
				}
			}
		nextTriangle:;
		}

		return neighbors;
	}

	struct Bounds
	{
		glm::vec3 min;
		glm::vec3 max;
	};

	Bounds getBounds() const
	{
		Bounds bounds;
		bounds.min = bounds.max = vertices[0];

		for (const auto &v : vertices)
		{
			bounds.min = glm::min(bounds.min, v);
			bounds.max = glm::max(bounds.max, v);
		}
		return bounds;
	}

private:
	std::vector<glm::vec3> vertices;
	std::vector<NavTriangle> triangles;
	bool isWalkableSurface(const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3, float max_slope_angle) const;
	void buildNeighborConnections();
	bool isPointInTriangle(const glm::vec3 &p, const NavTriangle &triangle) const;
};
