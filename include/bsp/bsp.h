// bsp.h
#pragma once
#include "raylib.h"
#include "raymath.h"
#include <memory>
#include <optional>
#include <vector>

// #define EPSILON 0.001f

struct BSPPlane {
	Vector3 normal;
	float distance;
};

struct Polygon {
	std::vector<Vector3> vertices;
	Vector3 normal;

	// Helper function for vector dot product
	static float dot(const Vector3 &a, const Vector3 &b) {
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	std::pair<std::optional<Polygon>, std::optional<Polygon>>
	split(const BSPPlane &plane) const {
		std::vector<Vector3> front_verts, back_verts;
		// const float EPSILON = 0.001f;

		// Process each edge
		for (size_t i = 0; i < vertices.size(); i++) {
			const Vector3 &current = vertices[i];
			const Vector3 &next = vertices[(i + 1) % vertices.size()];

			// Calculate distances
			float d1 = dot(plane.normal, current) - plane.distance;
			float d2 = dot(plane.normal, next) - plane.distance;

			// Classify vertices
			if (d1 > EPSILON)
				front_verts.push_back(current);
			else if (d1 < -EPSILON)
				back_verts.push_back(current);
			else {
				front_verts.push_back(current);
				back_verts.push_back(current);
			}

			// Handle edge intersection
			if ((d1 > EPSILON && d2 < -EPSILON) || (d1 < -EPSILON && d2 > EPSILON)) {
				float t = d1 / (d1 - d2);
				Vector3 intersection = {
					current.x + (next.x - current.x) * t,
					current.y + (next.y - current.y) * t,
					current.z + (next.z - current.z) * t
				};
				front_verts.push_back(intersection);
				back_verts.push_back(intersection);
			}
		}

		// Create split polygons if valid
		return {
			front_verts.size() >= 3 ? std::optional<Polygon>({ front_verts, normal }) : std::nullopt,
			back_verts.size() >= 3 ? std::optional<Polygon>({ back_verts, normal }) : std::nullopt
		};
	}

	BSPPlane get_plane() const {
		// Calculate normal
		Vector3 edge1 = Vector3Subtract(vertices[1], vertices[0]);
		Vector3 edge2 = Vector3Subtract(vertices[2], vertices[0]);
		Vector3 normal = Vector3Normalize(Vector3CrossProduct(edge1, edge2));

		// Calculate distance
		float distance = dot(normal, vertices[0]);

		return { normal, distance };
	}
};

struct BSPNode {
	BSPPlane plane;
	std::vector<int> polygons; // Indices to polygons in this node
	std::unique_ptr<BSPNode> front; // In front of plane
	std::unique_ptr<BSPNode> back; // Behind plane
};

class BSPTree {
private:
	std::unique_ptr<BSPNode> root;
	std::vector<Polygon> polygons; // Store all polygons

public:
	void build(const Mesh &mesh);
	bool trace_ray(const Vector3 &start, const Vector3 &direction, float &hit_distance);
	bool trace_node(BSPNode *node, const Vector3 &start, const Vector3 &direction, float &hit_distance);
	std::unique_ptr<BSPNode> build_node(const std::vector<Polygon> &node_polys, int depth);
	bool check_collision_node(BSPNode *node, const Vector3 &position, float radius);
	bool check_collision(const Vector3 &position, float radius);
	BSPPlane choose_split_plane(const std::vector<Polygon> &polygons);
	std::vector<Vector3> find_path(const Vector3 &start, const Vector3 &end);
	float dot_product(const Vector3 &a, const Vector3 &b);
};
