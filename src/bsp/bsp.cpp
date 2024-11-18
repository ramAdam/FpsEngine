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
			Vector3{ mesh.vertices[(i + 1) * 3], mesh.vertices[(i + 1) * 3 + 1], mesh.vertices[(i + 1) * 3 + 2] },
			Vector3{ mesh.vertices[(i + 2) * 3], mesh.vertices[(i + 2) * 3 + 1], mesh.vertices[(i + 2) * 3 + 2] }
		};

		// Use the normal of the first vertex for the entire polygon (assuming flat shading)
		poly.normal = Vector3{ mesh.normals[i * 3], mesh.normals[i * 3 + 1], mesh.normals[i * 3 + 2] };

		polygons.push_back(poly);
	}

	std::cout << "Total number of polygons: " << polygons.size() << std::endl;

	// Uncomment and implement if you want to build the BSP tree
	// root = build_node(polygons, 0);
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
			node->polygons.push_back(poly);
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

void BSPTree::collect_polygons_node(const BSPNode *node, std::vector<Polygon> &front_polys, std::vector<Polygon> &back_polys) const {
	if (!node)
		return;

	std::cout << "Polygons in node: " << node->polygons.size() << std::endl;

	// Collect polygons from current node
	for (const auto &poly : node->polygons) {
		if (dot_product(node->plane.normal, poly.vertices[0]) >= node->plane.distance) {
			front_polys.push_back(poly);
			std::cout << "Adding Front poly" << std::endl;
		} else {
			back_polys.push_back(poly);
		}
	}

	// Recursively collect from child nodes
	collect_polygons_node(node->front.get(), front_polys, back_polys);
	collect_polygons_node(node->back.get(), front_polys, back_polys);
}

void BSPTree::collect_polygons(std::vector<Polygon> &front_polys, std::vector<Polygon> &back_polys) const {
	if (root) {
		collect_polygons_node(root.get(), front_polys, back_polys);
	} else {
		std::cerr << "BSP tree is not built or root is null" << std::endl;
	}
}

float BSPTree::dot_product(const Vector3 &a, const Vector3 &b) const {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
