#pragma once
#include "raylib.h" // Add RayLib include
#include <glm/glm.hpp>
#include <vector>

struct NavTriangle
{
	int vertices[3];
	int neighbors[3]; // Adjacent triangles
};

class NavigationMesh
{
public:
	NavigationMesh();
	void buildFromMesh(const Model &model, float max_slope_angle = 45.0f);
	int find_nearest_triangle(const glm::vec3 &point) const;
	const NavTriangle &get_triangle(int index) const { return triangles[index]; }
	const glm::vec3 &get_vertex(int index) const { return vertices[index]; }

	const std::vector<NavTriangle> &getTriangles() const { return triangles; }
	const std::vector<glm::vec3> &getVertices() const { return vertices; }

private:
	std::vector<glm::vec3> vertices;
	std::vector<NavTriangle> triangles;
	bool isWalkableSurface(const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3, float max_slope_angle) const;
	void buildNeighborConnections();
	bool isPointInTriangle(const glm::vec3 &p, const NavTriangle &triangle) const;
};
