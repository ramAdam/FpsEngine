#include "navigation_mesh.h"
#include <iostream>

NavigationMesh::NavigationMesh() {}

void NavigationMesh::buildFromMesh(const Model &model, float max_slope_angle)
{
	vertices.clear();
	triangles.clear();

	std::cout << "Building navigation mesh...\n";

	// Process each mesh in the model
	for (int m = 0; m < model.meshCount; m++)
	{
		const Mesh &mesh = model.meshes[m];
		const float *verts = mesh.vertices;

		if (!verts)
		{
			std::cerr << "Mesh " << m << " has no vertices!\n";
			continue;
		}

		size_t baseVertex = vertices.size();

		// Convert vertices to our format
		for (int i = 0; i < mesh.vertexCount * 3; i += 3)
		{
			vertices.push_back(glm::vec3(verts[i], verts[i + 1], verts[i + 2]));
		}

		// Process triangles
		if (mesh.indices)
		{
			// Indexed mesh
			for (int i = 0; i < mesh.triangleCount * 3; i += 3)
			{
				int idx1 = mesh.indices[i];
				int idx2 = mesh.indices[i + 1];
				int idx3 = mesh.indices[i + 2];

				// Check walkable surface
				if (isWalkableSurface(
						vertices[baseVertex + idx1],
						vertices[baseVertex + idx2],
						vertices[baseVertex + idx3],
						max_slope_angle))
				{
					NavTriangle tri;
					tri.vertices[0] = baseVertex + idx1;
					tri.vertices[1] = baseVertex + idx2;
					tri.vertices[2] = baseVertex + idx3;
					tri.neighbors[0] = tri.neighbors[1] = tri.neighbors[2] = -1;
					triangles.push_back(tri);
				}
			}
		}
		else
		{
			// Non-indexed mesh
			for (int i = 0; i < mesh.vertexCount; i += 3)
			{
				if (isWalkableSurface(
						vertices[baseVertex + i],
						vertices[baseVertex + i + 1],
						vertices[baseVertex + i + 2],
						max_slope_angle))
				{
					NavTriangle tri;
					tri.vertices[0] = baseVertex + i;
					tri.vertices[1] = baseVertex + i + 1;
					tri.vertices[2] = baseVertex + i + 2;
					tri.neighbors[0] = tri.neighbors[1] = tri.neighbors[2] = -1;
					triangles.push_back(tri);
				}
			}
		}
	}

	std::cout << "Created " << vertices.size() << " vertices and "
			  << triangles.size() << " walkable triangles\n";

	// Build neighbor connections after all triangles are processed
	buildNeighborConnections();
}

bool NavigationMesh::isWalkableSurface(const glm::vec3 &v1, const glm::vec3 &v2, const glm::vec3 &v3, float max_slope_angle) const
{
	// Calculate triangle normal
	glm::vec3 edge1 = v2 - v1;
	glm::vec3 edge2 = v3 - v1;
	glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

	// Calculate angle between normal and up vector
	float angle = glm::degrees(glm::acos(glm::dot(normal, glm::vec3(0, 1, 0))));

	// Surface is walkable if angle is less than max slope angle
	return angle <= max_slope_angle;
}

void NavigationMesh::buildNeighborConnections()
{
	// For each triangle
	for (size_t i = 0; i < triangles.size(); i++)
	{
		// For each edge of current triangle
		for (int edge = 0; edge < 3; edge++)
		{
			if (triangles[i].neighbors[edge] != -1)
				continue;

			int v1 = triangles[i].vertices[edge];
			int v2 = triangles[i].vertices[(edge + 1) % 3];

			// Look for matching edge in other triangles
			for (size_t j = i + 1; j < triangles.size(); j++)
			{
				for (int e = 0; e < 3; e++)
				{
					int tv1 = triangles[j].vertices[e];
					int tv2 = triangles[j].vertices[(e + 1) % 3];

					if ((v1 == tv2 && v2 == tv1) || (v1 == tv1 && v2 == tv2))
					{
						triangles[i].neighbors[edge] = j;
						triangles[j].neighbors[e] = i;
						break;
					}
				}
			}
		}
	}
}

int NavigationMesh::findNearestTriangle(const glm::vec3 &point) const
{
	float nearestDist = std::numeric_limits<float>::max();
	int nearestTri = -1;

	for (size_t i = 0; i < triangles.size(); ++i)
	{
		const auto &tri = triangles[i];
		glm::vec3 center = (vertices[triangles[i].vertices[0]] + vertices[triangles[i].vertices[1]] + vertices[triangles[i].vertices[2]]) / 3.0f;

		float dist = glm::distance(point, center);
		if (dist < nearestDist)
		{
			// Check if point is above triangle and within reasonable height
			float height = point.y - center.y;
			if (height >= -1.0f && height <= 2.0f)
			{
				nearestDist = dist;
				nearestTri = i;
			}
		}
	}

	return nearestTri;
}

bool NavigationMesh::isPointInTriangle(const glm::vec3 &p, const NavTriangle &triangle) const
{
	const glm::vec3 &a = vertices[triangle.vertices[0]];
	const glm::vec3 &b = vertices[triangle.vertices[1]];
	const glm::vec3 &c = vertices[triangle.vertices[2]];

	// Compute barycentric coordinates
	glm::vec3 v0 = b - a;
	glm::vec3 v1 = c - a;
	glm::vec3 v2 = p - a;

	float d00 = glm::dot(v0, v0);
	float d01 = glm::dot(v0, v1);
	float d11 = glm::dot(v1, v1);
	float d20 = glm::dot(v2, v0);
	float d21 = glm::dot(v2, v1);

	float denom = d00 * d11 - d01 * d01;
	float v = (d11 * d20 - d01 * d21) / denom;
	float w = (d00 * d21 - d01 * d20) / denom;
	float u = 1.0f - v - w;

	return v >= 0.0f && w >= 0.0f && (v + w) <= 1.0f;
}
