#include "qodot.h"

#include "geo_generator.h"
#include "map_data.h"
#include "map_parser.h"
#include "surface_gatherer.h"
#include <iostream> // for std::cerr
#include <stdexcept> // for std::runtime_error

using namespace std;

void Qodot::load_map(const string &map_file) {
	map_parser.map_parser_load(map_file.c_str());
}

std::vector<std::string> Qodot::get_texture_list() {
	std::vector<std::string> textures;

	// Validate map data
	if (!map_data) {
		throw std::runtime_error("map_data is null");
	}

	int tex_count = map_data->map_data_get_texture_count();
	if (tex_count <= 0) {
		return textures;
	}

	LMTextureData *texture_data = map_data->map_data_get_textures();
	if (!texture_data) {
		throw std::runtime_error("Failed to get textures");
	}

	textures.reserve(tex_count);

	for (int i = 0; i < tex_count; i++) {
		if (texture_data[i].name) {
			textures.push_back(std::string(texture_data[i].name));
		}
	}

	return textures;
}

void Qodot::set_entity_definitions(const std::unordered_map<std::string, ENTITY_SPAWN_TYPE> &entity_defs) {
	try {
		for (const auto &[classname, spawn_type] : entity_defs) {
			if (classname.empty()) {
				continue;
			}
			map_parser.map_data->map_data_set_spawn_type_by_classname(
					classname.c_str(),
					static_cast<int>(spawn_type));
		}
	} catch (const std::exception &e) {
		throw std::runtime_error("Failed to set entity definitions: " + std::string(e.what()));
	}
}

void Qodot::set_worldspawn_layers(const std::vector<WorldspawnLayer> &worldspawn_layers) {
	try {
		for (const auto &layer : worldspawn_layers) {
			if (layer.texture.empty()) {
				continue;
			}

			map_data->map_data_register_worldspawn_layer(
					layer.texture.c_str(),
					layer.build_visuals);
		}
	} catch (const std::exception &e) {
		throw std::runtime_error("Failed to set worldspawn layers: " + std::string(e.what()));
	}
}

void Qodot::generate_geometry(std::unordered_map<std::string, TextureSize> texture_dict) {
	// Iterate through map entries
	for (const auto &[texture_key, size] : texture_dict) {
		int width = size.width;
		int height = size.height;

		// Pass raw C string to map_data
		map_data->map_data_set_texture_size(texture_key.c_str(), width, height);
	}

	geo_generator.geo_generator_run();
}

std::vector<EntityData> Qodot::get_entity_dicts() {
	try {
		int ent_count = map_data->map_data_get_entity_count();
		const LMEntity *ents = map_data->map_data_get_entities();

		std::vector<EntityData> entity_dicts;
		entity_dicts.reserve(ent_count);

		for (int i = 0; i < ent_count; i++) {
			const LMEntity *ent = &ents[i];
			EntityData entity_data{
				ent->brush_count,
				{},
				Vector3(ent->center.y, ent->center.z, ent->center.x),
				{}
			};

			// Process brush indices
			for (int b = 0; b < ent->brush_count; b++) {
				const LMBrush *brush = &ent->brushes[b];
				bool is_worldspawn_layer_brush = false;

				for (int f = 0; f < brush->face_count; f++) {
					const face *face = &brush->faces[f];
					if (map_data->map_data_find_worldspawn_layer(face->texture_idx) != -1) {
						is_worldspawn_layer_brush = true;
						break;
					}
				}

				if (!is_worldspawn_layer_brush) {
					entity_data.brush_indices.push_back(b);
				}
			}

			// Process properties
			for (int p = 0; p < ent->property_count; p++) {
				LMProperty *prop = &ent->properties[p];
				entity_data.properties[prop->key] = prop->value;
			}

			entity_dicts.push_back(std::move(entity_data));
		}

		return entity_dicts;
	} catch (const std::exception &e) {
		throw std::runtime_error("Failed to get entity dictionaries: " + std::string(e.what()));
	}
}

std::vector<WorldspawnLayerData> Qodot::get_worldspawn_layer_dicts() {
	std::vector<WorldspawnLayerData> worldspawn_layer_dicts;

	try {
		const LMEntity *ents = map_data->map_data_get_entities();
		const LMEntity *worldspawn_entity = &ents[0];

		if (worldspawn_entity == nullptr) {
			return worldspawn_layer_dicts;
		}

		int layer_count = map_data->map_data_get_worldspawn_layer_count();
		const LMWorldspawnLayer *layers = map_data->map_data_get_worldspawn_layers();

		worldspawn_layer_dicts.reserve(layer_count);

		for (int l = 0; l < layer_count; l++) {
			const LMWorldspawnLayer *worldspawn_layer = &layers[l];
			WorldspawnLayerData layer_data;

			LMTextureData *tex_data = map_data->map_data_get_texture(worldspawn_layer->texture_idx);
			if (tex_data == nullptr) {
				continue;
			}

			layer_data.texture = tex_data->name;

			// Process brush indices
			for (int b = 0; b < worldspawn_entity->brush_count; ++b) {
				const LMBrush *brush = &worldspawn_entity->brushes[b];
				bool is_layer_brush = false;

				for (int f = 0; f < brush->face_count; ++f) {
					const face *face = &brush->faces[f];
					if (face->texture_idx == worldspawn_layer->texture_idx) {
						is_layer_brush = true;
						break;
					}
				}

				if (is_layer_brush) {
					layer_data.brush_indices.push_back(b);
				}
			}

			worldspawn_layer_dicts.push_back(std::move(layer_data));
		}
	} catch (const std::exception &e) {
		throw std::runtime_error("Failed to get worldspawn layer dictionaries: " + std::string(e.what()));
	}

	return worldspawn_layer_dicts;
}

void Qodot::gather_texture_surfaces(
		const std::string &texture_name,
		const std::string &brush_filter_texture,
		const std::string &face_filter_texture) {
	try {
		gather_texture_surfaces_internal(
				texture_name,
				brush_filter_texture,
				face_filter_texture,
				true);
	} catch (const std::exception &e) {
		throw std::runtime_error("Failed to gather texture surfaces: " + std::string(e.what()));
	}
}

void Qodot::gather_worldspawn_layer_surfaces(
		const std::string &texture_name,
		const std::string &brush_filter_texture,
		const std::string &face_filter_texture) {
	try {
		gather_texture_surfaces_internal(
				texture_name,
				brush_filter_texture,
				face_filter_texture,
				false);
	} catch (const std::exception &e) {
		throw std::runtime_error("Failed to gather worldspawn layer surfaces: " + std::string(e.what()));
	}
}

void Qodot::gather_texture_surfaces_internal(
		const std::string &texture_name,
		const std::string &brush_filter_texture,
		const std::string &face_filter_texture,
		bool filter_layers) {
	try {
		surface_gatherer.surface_gatherer_reset_params();
		surface_gatherer.surface_gatherer_set_split_type(SST_ENTITY);
		surface_gatherer.surface_gatherer_set_texture_filter(texture_name.c_str());
		surface_gatherer.surface_gatherer_set_brush_filter_texture(brush_filter_texture.c_str());
		surface_gatherer.surface_gatherer_set_face_filter_texture(face_filter_texture.c_str());
		surface_gatherer.surface_gatherer_set_worldspawn_layer_filter(filter_layers);

		surface_gatherer.surface_gatherer_run();
	} catch (const std::exception &e) {
		throw std::runtime_error("Failed to gather texture surfaces: " + std::string(e.what()));
	}
}

void Qodot::gather_entity_convex_collision_surfaces(size_t entity_idx) {
	try {
		gather_convex_collision_surfaces(entity_idx, true);
	} catch (const std::exception &e) {
		throw std::runtime_error("Failed to gather entity convex collision surfaces: " + std::string(e.what()));
	}
}

void Qodot::gather_convex_collision_surfaces(size_t p_entity_idx, bool p_filter_layers) {
	surface_gatherer.surface_gatherer_reset_params();
	surface_gatherer.surface_gatherer_set_split_type(SST_BRUSH);
	surface_gatherer.surface_gatherer_set_entity_index_filter((int)p_entity_idx);
	surface_gatherer.surface_gatherer_set_worldspawn_layer_filter(p_filter_layers);

	surface_gatherer.surface_gatherer_run();
}

void Qodot::gather_concave_collision_surfaces(size_t p_entity_idx, bool p_filter_layers) {
	surface_gatherer.surface_gatherer_reset_params();
	surface_gatherer.surface_gatherer_set_split_type(SST_NONE);
	surface_gatherer.surface_gatherer_set_entity_index_filter((int)p_entity_idx);
	surface_gatherer.surface_gatherer_set_worldspawn_layer_filter(p_filter_layers);

	surface_gatherer.surface_gatherer_run();
}

std::vector<MeshData> Qodot::fetch_surfaces(double inverse_scale_factor) {
	const LMSurfaces *surfs = surface_gatherer.surface_gatherer_fetch();
	std::vector<MeshData> surface_collection;

	if (!surfs) {
		return surface_collection;
	}

	for (int s = 0; s < surfs->surface_count; ++s) {
		LMSurface *surf = &surfs->surfaces[s];

		if (surf->vertex_count == 0) {
			continue;
		}

		MeshData mesh_data;

		// Convert vertices
		for (int v = 0; v < surf->vertex_count; ++v) {
			Vector3 vertex(
					surf->vertices[v].vertex.y,
					surf->vertices[v].vertex.z,
					surf->vertices[v].vertex.x);
			vertex = vertex / inverse_scale_factor;
			mesh_data.vertices.push_back(vertex);
		}

		// Convert normals
		for (int v = 0; v < surf->vertex_count; ++v) {
			Vector3 normal(
					surf->vertices[v].normal.y,
					surf->vertices[v].normal.z,
					surf->vertices[v].normal.x);
			mesh_data.normals.push_back(normal);
		}

		// Convert tangents
		for (int v = 0; v < surf->vertex_count; ++v) {
			Vector4 tangent(
					surf->vertices[v].tangent.y,
					surf->vertices[v].tangent.z,
					surf->vertices[v].tangent.x,
					surf->vertices[v].tangent.w);
			mesh_data.tangents.push_back(tangent);
		}

		// Convert UVs
		for (int v = 0; v < surf->vertex_count; ++v) {
			Vector2 uv(
					surf->vertices[v].uv.u,
					surf->vertices[v].uv.v);
			mesh_data.uvs.push_back(uv);
		}

		// Convert indices
		for (int i = 0; i < surf->index_count; ++i) {
			mesh_data.indices.push_back(surf->indices[i]);
		}

		surface_collection.push_back(mesh_data);
	}

	return surface_collection;
}
