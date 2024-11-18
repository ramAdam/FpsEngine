#pragma once
#include "raylib.h"
#include "raymath.h"
#include <memory>
#include <optional>
#include <vector>

struct BSPPlane {
	Vector3 normal; // Assumed to be normalized
	float distance;

	float signed_distance_to_point(const Vector3 &point) const {
		return dot_product(normal, point) - distance;
	}

private:
	static float dot_product(const Vector3 &a, const Vector3 &b) {
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}
};

struct Polygon {
	std::vector<Vector3> vertices;
	Vector3 normal;

	static float dot_product(const Vector3 &a, const Vector3 &b) {
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	BSPPlane get_plane() const {
		return BSPPlane{ normal, dot_product(normal, vertices[0]) };
	}

	std::pair<std::optional<Polygon>, std::optional<Polygon>>
	split(const BSPPlane &plane) const {
		std::vector<Vector3> front_vertices, back_vertices;

		for (size_t i = 0; i < vertices.size(); ++i) {
			const Vector3 &current_vertex = vertices[i];
			const Vector3 &next_vertex = vertices[(i + 1) % vertices.size()];

			float current_distance = plane.signed_distance_to_point(current_vertex);
			float next_distance = plane.signed_distance_to_point(next_vertex);

			if (current_distance >= 0) {
				front_vertices.push_back(current_vertex);
			} else {
				back_vertices.push_back(current_vertex);
			}

			if ((current_distance >= 0 && next_distance < 0) || (current_distance < 0 && next_distance >= 0)) {
				float t = current_distance / (current_distance - next_distance);
				Vector3 intersection = compute_intersection(current_vertex, next_vertex, t);
				front_vertices.push_back(intersection);
				back_vertices.push_back(intersection);
			}
		}

		std::optional<Polygon> front_poly, back_poly;
		if (!front_vertices.empty()) {
			front_poly.emplace(Polygon{ front_vertices, normal });
		}
		if (!back_vertices.empty()) {
			back_poly.emplace(Polygon{ back_vertices, normal });
		}

		return { front_poly, back_poly };
	}

private:
	Vector3 compute_intersection(const Vector3 &a, const Vector3 &b, float t) const {
		return Vector3Lerp(a, b, t); // Assuming Vector3Lerp is defined
	}
};

struct BSPNode {
	BSPPlane plane;
	std::vector<Polygon> polygons;
	std::unique_ptr<BSPNode> front;
	std::unique_ptr<BSPNode> back;
};

class BSPTree {
public:
	std::unique_ptr<BSPNode> root;
	std::vector<Polygon> polygons;
	void build(const Mesh &mesh);
	void collect_polygons(std::vector<Polygon> &front_polys, std::vector<Polygon> &back_polys) const;

private:
	std::unique_ptr<BSPNode> build_node(const std::vector<Polygon> &node_polys, int depth);
	BSPPlane choose_split_plane(const std::vector<Polygon> &polygons);
	void collect_polygons_node(const BSPNode *node, std::vector<Polygon> &front_polys, std::vector<Polygon> &back_polys) const;
	float dot_product(const Vector3 &a, const Vector3 &b) const;
};
