#include "map_loader.h"
#include <raymath.h>

// vector from raylib

bool MapLoader::load(const std::string &mapPath) {
	LMMapParser parser = LMMapParser(std::make_shared<LMMapData>());

	// Load the map file
	if (!parser.map_parser_load(mapPath.c_str())) {
		return false;
	}

	// Process loaded data
	m_isLoaded = processMapData(parser.map_data);
	return m_isLoaded;
}

bool MapLoader::processMapData(const std::shared_ptr<LMMapData> &mapData) {
	m_meshes.clear();

	// Process each entity
	for (int entity_idx = 0; entity_idx < mapData->entity_count; entity_idx++) {
		const LMEntity &entity = mapData->entities[entity_idx];
		const LMEntityGeometry &entity_geo = mapData->entity_geo[entity_idx];

		// Process each brush in entity
		for (int brush_idx = 0; brush_idx < entity.brush_count; brush_idx++) {
			MeshData meshData;
			const LMBrush &brush = entity.brushes[brush_idx];

			// Process each face in brush
			for (int face_idx = 0; face_idx < brush.face_count; face_idx++) {
				const LMFace &face = brush.faces[face_idx];

				// Get base vertex index for this face
				size_t base_idx = meshData.vertices.size();

				// Add vertices from plane points with proper conversion
				meshData.vertices.push_back(Vector3{
						static_cast<float>(face.plane_points.v0.x),
						static_cast<float>(face.plane_points.v0.y),
						static_cast<float>(face.plane_points.v0.z) });
				meshData.vertices.push_back(Vector3{
						static_cast<float>(face.plane_points.v1.x),
						static_cast<float>(face.plane_points.v1.y),
						static_cast<float>(face.plane_points.v1.z) });
				meshData.vertices.push_back(Vector3{
						static_cast<float>(face.plane_points.v2.x),
						static_cast<float>(face.plane_points.v2.y),
						static_cast<float>(face.plane_points.v2.z) });

				// Calculate face normal from plane points
				Vector3 normal = calculateNormal(face.plane_points);
				meshData.normals.push_back(normal);
				meshData.normals.push_back(normal);
				meshData.normals.push_back(normal);

				// Add UV coordinates
				if (face.is_valve_uv) {
					// Handle Valve UV format
					Vector2 uv0 = calculateValveUV(face.uv_valve, face.plane_points.v0);
					Vector2 uv1 = calculateValveUV(face.uv_valve, face.plane_points.v1);
					Vector2 uv2 = calculateValveUV(face.uv_valve, face.plane_points.v2);
					meshData.uvs.push_back(uv0);
					meshData.uvs.push_back(uv1);
					meshData.uvs.push_back(uv2);
				} else {
					// Handle standard UV format
					meshData.uvs.push_back(makeUV(face.uv_standard.u, face.uv_standard.v));
					meshData.uvs.push_back(makeUV(face.uv_standard.u + face.uv_extra.scale_x, face.uv_standard.v));
					meshData.uvs.push_back(makeUV(face.uv_standard.u, face.uv_standard.v + face.uv_extra.scale_y));
				}

				// Add indices for triangulation
				meshData.indices.push_back(base_idx);
				meshData.indices.push_back(base_idx + 1);
				meshData.indices.push_back(base_idx + 2);
			}

			// Verify mesh before adding to collection
			if (!meshData.vertices.empty()) {
				verify_winding_order(mapData, meshData);
				verify_vertex_positions(meshData);
				m_meshes.push_back(meshData);
			}
		}
	}

	return !m_meshes.empty();
}

void MapLoader::verify_winding_order(const std::shared_ptr<LMMapData> &mapData, const MeshData &mesh) {
	if (!mapData) {
		TraceLog(LOG_WARNING, "Invalid map data provided");
		return;
	}

	for (int entity_idx = 0; entity_idx < mapData->entity_count; entity_idx++) {
		const LMEntity &entity = mapData->entities[entity_idx];
		TraceLog(LOG_DEBUG, "Verifying entity %d", entity_idx);

		for (int brush_idx = 0; brush_idx < entity.brush_count; brush_idx++) {
			const LMBrush &brush = entity.brushes[brush_idx];
			for (int face_idx = 0; face_idx < brush.face_count; face_idx++) {
				log_face_indices(face_idx, mesh.indices);
			}
		}
	}
}

void MapLoader::verify_vertex_positions(const MeshData &mesh) {
	if (mesh.vertices.empty()) {
		TraceLog(LOG_WARNING, "Empty mesh provided for verification");
		return;
	}

	for (size_t i = 0; i < mesh.vertices.size(); i++) {
		const auto &vertex = mesh.vertices[i];
		TraceLog(LOG_DEBUG, "Vertex %zu: (%.2f, %.2f, %.2f)",
				i, vertex.x, vertex.y, vertex.z);
	}
}

void MapLoader::log_face_indices(int face_idx, const std::vector<int> &indices) {
	TraceLog(LOG_DEBUG, "Face %d indices: %d, %d, %d",
			face_idx,
			indices[face_idx * 3],
			indices[face_idx * 3 + 1],
			indices[face_idx * 3 + 2]);
}

// Helper function to calculate normal from three points
Vector3 MapLoader::calculateNormal(const LMFacePoints &points) {
	Vector3 v1 = {
		points.v1.x - points.v0.x,
		points.v1.y - points.v0.y,
		points.v1.z - points.v0.z
	};
	Vector3 v2 = {
		points.v2.x - points.v0.x,
		points.v2.y - points.v0.y,
		points.v2.z - points.v0.z
	};

	// Cross product to get normal
	return Vector3Normalize({
			v1.y * v2.z - v1.z * v2.y, // x component
			v1.z * v2.x - v1.x * v2.z, // y component
			v1.x * v2.y - v1.y * v2.x // z component
	});
}

// Helper function for Valve UV calculation
Vector2 MapLoader::calculateValveUV(const LMValveUV &uv, const vec3 &vertex) {
	float u = uv.u.axis.x * vertex.x + uv.u.axis.y * vertex.y + uv.u.axis.z * vertex.z + uv.u.offset;
	float v = uv.v.axis.x * vertex.x + uv.v.axis.y * vertex.y + uv.v.axis.z * vertex.z + uv.v.offset;
	return { u, v };
}

// Helper function to make code more readable
Vector2 MapLoader::makeUV(float u, float v) {
	return Vector2{ u, v };
}

Vector3 calculateFaceNormal(const Vector3 &v0, const Vector3 &v1, const Vector3 &v2) {
	Vector3 edge1 = {
		v1.x - v0.x,
		v1.y - v0.y,
		v1.z - v0.z
	};
	Vector3 edge2 = {
		v2.x - v0.x,
		v2.y - v0.y,
		v2.z - v0.z
	};
	// Cross product
	Vector3 normal = {
		edge1.y * edge2.z - edge1.z * edge2.y,
		edge1.z * edge2.x - edge1.x * edge2.z,
		edge1.x * edge2.y - edge1.y * edge2.x
	};
	return Vector3Normalize(normal);
}