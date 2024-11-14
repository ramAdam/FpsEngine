#ifndef QODOT_H
#define QODOT_H

#include "geo_generator.h"
#include "map_parser.h"
#include "surface_gatherer.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

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
	void set_entity_definitions(std::unordered_map<std::string, std::string> entity_defs);
	void set_worldspawn_layers(std::vector<std::string> worldspawn_layers);
	void generate_geometry(std::unordered_map<std::string, std::string> texture_dict);

	// Entity related
	std::vector<std::unordered_map<std::string, std::string>> get_entity_dicts();
	std::vector<std::unordered_map<std::string, std::string>> get_worldspawn_layer_dicts();

	// Surface gathering
	void gather_texture_surfaces(const std::string &texture_name,
			const std::string &brush_filter_texture,
			const std::string &face_filter_texture);

	void gather_worldspawn_layer_surfaces(const std::string &texture_name,
			const std::string &brush_filter_texture,
			const std::string &face_filter_texture);

	void gather_entity_convex_collision_surfaces(size_t entity_idx);
	void gather_entity_concave_collision_surfaces(size_t entity_idx);
	void gather_worldspawn_layer_collision_surfaces(size_t entity_idx);

	std::vector<float> fetch_surfaces(double inverse_scale_factor);

private:
	void gather_texture_surfaces_internal(const std::string &texture_name,
			const std::string &brush_filter_texture,
			const std::string &face_filter_texture,
			bool filter_layers);
};

#endif // QODOT_H