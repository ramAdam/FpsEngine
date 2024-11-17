#include "bsp.h"
#include <optional>
#include <stdexcept>
#include <utility>

void BSPTree::build(const Mesh &mesh) {
	if (!mesh.vertices || !mesh.indices || !mesh.normals) {
		throw std::runtime_error("Invalid mesh data");
	}

	// Convert mesh triangles to polygons
	for (int i = 0; i < mesh.triangleCount * 3; i += 3) {
		Polygon poly;

		// Calculate vertex indices
		unsigned short idx0 = mesh.indices[i];
		unsigned short idx1 = mesh.indices[i + 1];
		unsigned short idx2 = mesh.indices[i + 2];

		// Each vertex has 3 components (XYZ)
		poly.vertices = {
			Vector3{
					mesh.vertices[idx0 * 3],
					mesh.vertices[idx0 * 3 + 1],
					mesh.vertices[idx0 * 3 + 2] },
			Vector3{
					mesh.vertices[idx1 * 3],
					mesh.vertices[idx1 * 3 + 1],
					mesh.vertices[idx1 * 3 + 2] },
			Vector3{
					mesh.vertices[idx2 * 3],
					mesh.vertices[idx2 * 3 + 1],
					mesh.vertices[idx2 * 3 + 2] }
		};

		// Each normal has 3 components (XYZ)
		poly.normal = Vector3{
			mesh.normals[idx0 * 3],
			mesh.normals[idx0 * 3 + 1],
			mesh.normals[idx0 * 3 + 2]
		};

		polygons.push_back(poly);
	}

	root = build_node(polygons);
}

bool BSPTree::trace_ray(const Vector3 &start, const Vector3 &direction, float &hit_distance) {
	return trace_node(root.get(), start, direction, hit_distance);
}

bool BSPTree::trace_node(BSPNode *node, const Vector3 &start, const Vector3 &direction, float &hit_distance) {
	if (!node)
		return false;

	float dist = dot_product(node->plane.normal, start) - node->plane.distance;
	float dot = dot_product(node->plane.normal, direction);

	BSPNode *near_node, *far_node;
	if (dist >= 0) {
		near_node = node->front.get();
		far_node = node->back.get();
	} else {
		near_node = node->back.get();
		far_node = node->front.get();
	}

	// Check near side
	if (trace_node(near_node, start, direction, hit_distance))
		return true;

	// Check polygons in current node
	for (const auto &poly_idx : node->polygons) {
		const auto &poly = polygons[poly_idx];
		// Implement triangle intersection test here
		// If hit found, update hit_distance and return true
	}

	// Check far side
	return trace_node(far_node, start, direction, hit_distance);
}

bool BSPTree::check_collision(const Vector3 &position, float radius) {
	return check_collision_node(root.get(), position, radius);
}

bool BSPTree::check_collision_node(BSPNode *node, const Vector3 &position, float radius) {
	if (!node)
		return false;

	float dist = dot_product(node->plane.normal, position) - node->plane.distance;

	// Check both sides if sphere intersects plane
	if (std::abs(dist) <= radius) {
		if (check_collision_node(node->front.get(), position, radius))
			return true;
		if (check_collision_node(node->back.get(), position, radius))
			return true;

		// Check polygons in current node
		for (const auto &poly_idx : node->polygons) {
			const auto &poly = polygons[poly_idx];
			// Implement sphere-triangle collision test here
		}
		return false;
	}

	// Only check appropriate side
	return check_collision_node(
			(dist >= 0) ? node->front.get() : node->back.get(),
			position, radius);
}

std::unique_ptr<BSPNode> BSPTree::build_node(const std::vector<Polygon> &node_polys) {
	if (node_polys.empty())
		return nullptr;

	auto node = std::make_unique<BSPNode>();

	// Choose best splitting plane
	node->plane = choose_split_plane(node_polys);

	std::vector<Polygon> front_polys, back_polys;

	// Store indices of polygons that lie on the splitting plane
	for (size_t i = 0; i < node_polys.size(); i++) {
		const auto &poly = node_polys[i];
		const auto &[front_opt, back_opt] = poly.split(node->plane);

		// Use optional values
		if (!front_opt && !back_opt) {
			// Polygon lies on plane
			node->polygons.push_back(polygons.size());
			polygons.push_back(poly);
		} else {
			if (front_opt) {
				front_polys.push_back(*front_opt);
			}
			if (back_opt) {
				back_polys.push_back(*back_opt);
			}
		}
	}

	// Recursively build child nodes
	node->front = build_node(front_polys);
	node->back = build_node(back_polys);

	return node;
}

BSPPlane BSPTree::choose_split_plane(const std::vector<Polygon> &polygons) {
	if (polygons.empty()) {
		return BSPPlane(); // Return default plane
	}

	// Simple heuristic: choose polygon that splits fewest others
	BSPPlane best_plane;
	int best_score = std::numeric_limits<int>::max();

	for (const auto &poly : polygons) {
		BSPPlane plane = poly.get_plane();
		int score = 0;

		for (const auto &other : polygons) {
			auto [front, back] = other.split(plane);
			if (front && back)
				score++; // Count splits
		}

		if (score < best_score) {
			best_score = score;
			best_plane = plane;
		}
	}

	return best_plane;
}

std::vector<Vector3> BSPTree::find_path(const Vector3 &start, const Vector3 &end) {
	return std::vector<Vector3>();
}

float dot_product(const Vector3 &a, const Vector3 &b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
