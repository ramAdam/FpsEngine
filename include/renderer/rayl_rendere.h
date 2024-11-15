class RaylibRenderer {
private:
	// Cache for uploaded meshes using a hash or ID
	std::unordered_map<size_t, Mesh> mesh_cache;

	// Helper to generate mesh hash/ID
	size_t generate_mesh_id(const MeshData &mesh) {
		// Simple hash combining vertex count and indices
		return std::hash<size_t>{}(mesh.vertices.size()) ^
			   std::hash<size_t>{}(mesh.indices.size());
	}

public:
	void render_mesh(const MeshData &mesh) {
		size_t mesh_id = generate_mesh_id(mesh);

		// Check if mesh is already uploaded
		if (mesh_cache.find(mesh_id) == mesh_cache.end()) {
			// New mesh - upload to GPU
			Mesh rlMesh = { 0 };
			rlMesh.vertexCount = static_cast<int>(mesh.vertices.size());
			rlMesh.triangleCount = static_cast<int>(mesh.indices.size() / 3);

			if (!mesh.vertices.empty())
				rlMesh.vertices = (float *)mesh.vertices.data();
			if (!mesh.normals.empty())
				rlMesh.normals = (float *)mesh.normals.data();
			if (!mesh.uvs.empty())
				rlMesh.texcoords = (float *)mesh.uvs.data();

			UploadMesh(&rlMesh, false);
			mesh_cache[mesh_id] = rlMesh;
		}

		// Draw cached mesh
	}
};