#ifndef QODOT_H
#define QODOT_H

#include "geo_generator.h"
#include "map_parser.h"
#include "raylib.h"
#include "surface_gatherer.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct WorldspawnLayer {
	std::string texture;
	bool build_visuals;
};

struct TextureSize {
	int width;
	int height;
};

// struct Vector3 {
// 	float x, y, z;
// 	Vector3(float x_, float y_, float z_) :
// 			x(x_), y(y_), z(z_) {}
// };

// Required structs
// Remove Vector2, Vector3, Vector4 structs as we'll use raylib's

struct MeshData {
	std::vector<Vector3> vertices;
	std::vector<Vector3> normals;
	std::vector<Vector4> tangents;
	std::vector<Vector2> uvs;
	std::vector<int> indices;
};

struct EntityData {
	int brush_count;
	std::vector<int> brush_indices;
	Vector3 center;
	std::unordered_map<std::string, std::string> properties;
};

struct WorldspawnLayerData {
	std::string texture;
	std::vector<int> brush_indices;
};

class Qodot {
private:
	std::shared_ptr<LMMapData> map_data = std::make_shared<LMMapData>();
	LMMapParser map_parser;
	LMGeoGenerator geo_generator;
	LMSurfaceGatherer surface_gatherer;

public:
	Qodot() :
			map_parser(map_data),
			geo_generator(map_data),
			surface_gatherer(map_data) {}

	// Core functionality
	void load_map(const std::string &map_file);
	std::vector<std::string> get_texture_list();
	void set_entity_definitions(const std::unordered_map<std::string, ENTITY_SPAWN_TYPE> &entity_defs);
	void set_worldspawn_layers(const std::vector<WorldspawnLayer> &worldspawn_layers);
	void generate_geometry(std::unordered_map<std::string, TextureSize> texture_dict);

	std::vector<EntityData> get_entity_dicts();
	std::vector<WorldspawnLayerData> get_worldspawn_layer_dicts();

	// Surface gathering
	void gather_texture_surfaces(const std::string &texture_name,
			const std::string &brush_filter_texture,
			const std::string &face_filter_texture);

	void gather_worldspawn_layer_surfaces(const std::string &texture_name,
			const std::string &brush_filter_texture,
			const std::string &face_filter_texture);

	void gather_convex_collision_surfaces(size_t p_entity_idx, bool p_filter_layers);
	void gather_concave_collision_surfaces(size_t p_entity_idx, bool p_filter_layers);
	void gather_worldspawn_layer_collision_surfaces(size_t entity_idx);
	void gather_entity_convex_collision_surfaces(size_t entity_idx);
	void gather_entity_concave_collision_surfaces(size_t entity_idx);

	std::vector<MeshData> fetch_surfaces(double inverse_scale_factor);

private:
	void gather_texture_surfaces_internal(const std::string &texture_name,
			const std::string &brush_filter_texture,
			const std::string &face_filter_texture,
			bool filter_layers);
};

#endif // QODOT_H
