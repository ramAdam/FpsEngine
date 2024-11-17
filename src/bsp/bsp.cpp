#include "bsp.h"
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>

void BSPTree::build(const Mesh &mesh) {
	if (!mesh.vertices || !mesh.normals) {
		throw std::runtime_error("Invalid mesh data: missing vertices or normals");
	}

	for (int i = 0; i < mesh.vertexCount; i += 3) {
		Polygon poly;
		poly.vertices = {
			Vector3{ mesh.vertices[i * 3], mesh.vertices[i * 3 + 1], mesh.vertices[i * 3 + 2] },
			Vector3{ mesh.vertices[(i + 1) * 3], mesh.vertices[(i + 1) * 3 + 1], mesh.vertices[(i + 2) * 3 + 2] },
			Vector3{ mesh.vertices[(i + 2) * 3], mesh.vertices[(i + 2) * 3 + 1], mesh.vertices[(i + 2) * 3 + 2] }
		};
		poly.normal = Vector3{ mesh.normals[i * 3], mesh.normals[i * 3 + 1], mesh.normals[i * 3 + 2] };
		polygons.push_back(poly);
	}

	std::cout << "Total number of polygons: " << polygons.size() << std::endl;

	root = build_node(polygons, 0);
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

std::unique_ptr<BSPNode> BSPTree::build_node(const std::vector<Polygon> &node_polys, int depth) {
	const int MAX_DEPTH = 20;
	const int MIN_POLYS = 1; // Base case for small number of polygons

	if (node_polys.empty() || depth >= MAX_DEPTH || node_polys.size() <= MIN_POLYS) {
		return nullptr;
	}

	auto node = std::make_unique<BSPNode>();
	node->plane = choose_split_plane(node_polys);

	std::vector<Polygon> front_polys, back_polys;
	for (const auto &poly : node_polys) {
		auto [front_opt, back_opt] = poly.split(node->plane);
		if (!front_opt && !back_opt) {
			node->polygons.push_back(polygons.size());
			polygons.push_back(poly);
		} else {
			if (front_opt)
				front_polys.push_back(*front_opt);
			if (back_opt)
				back_polys.push_back(*back_opt);
		}
	}

	std::cout << "Depth: " << depth << ", Front polys: " << front_polys.size() << ", Back polys: " << back_polys.size() << std::endl;

	// Base case to stop recursion if no meaningful split
	if (front_polys.size() == node_polys.size() || back_polys.size() == node_polys.size()) {
		std::cout << "Base case reached" << std::endl;
		return node;
	}

	node->front = build_node(front_polys, depth + 1);
	node->back = build_node(back_polys, depth + 1);

	return node;
}

BSPPlane BSPTree::choose_split_plane(const std::vector<Polygon> &polygons) {
	if (polygons.empty())
		return BSPPlane();

	BSPPlane best_plane;
	int best_score = std::numeric_limits<int>::max();

	for (const auto &poly : polygons) {
		BSPPlane plane = poly.get_plane();
		int score = 0;
		int front_count = 0;
		int back_count = 0;

		for (const auto &other : polygons) {
			auto [front, back] = other.split(plane);
			if (front && back)
				score++;
			if (front)
				front_count++;
			if (back)
				back_count++;
		}

		// Balance score: splits + abs(front - back)
		int balance_score = std::abs(front_count - back_count);
		int total_score = score + balance_score;

		if (total_score < best_score) {
			best_score = total_score;
			best_plane = plane;
		}
	}

	return best_plane;
}

std::vector<Vector3> BSPTree::find_path(const Vector3 &start, const Vector3 &end) {
	return std::vector<Vector3>();
}

float BSPTree::dot_product(const Vector3 &a, const Vector3 &b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
