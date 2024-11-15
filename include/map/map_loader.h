#pragma once
#include "face.h"
#include "map_parser.h"
#include "vector.h"
#include <raylib.h>
#include <memory>
#include <string>
#include <vector>

struct MeshData {
	std::vector<Vector3> vertices;
	std::vector<Vector3> normals;
	std::vector<Vector4> tangents;
	std::vector<Vector2> uvs;
	std::vector<int> indices;
};

class MapLoader {
public:
	MapLoader() = default;
	~MapLoader() = default;

	bool load(const std::string &mapPath);
	const std::vector<MeshData> &getMeshes() const { return m_meshes; }
	bool isLoaded() const { return m_isLoaded; }

private:
	bool processMapData(const std::shared_ptr<LMMapData> &mapData);
	Vector3 calculateNormal(const LMFacePoints &points);
	Vector2 calculateValveUV(const LMValveUV &uv, const vec3 &vertex);
	Vector2 makeUV(float u, float v);
	std::vector<MeshData> m_meshes;
	bool m_isLoaded{ false };
};
