///
///	\file		objloader.cpp
/// \brief		Contains implementation for the <see cref="HF::Geometry">Geometry</see> namespace
///
///	\author		TBA
///	\date		16 Jun 2020

#include <objloader.h>
#include <Dense>
#include <meshinfo.h>
#define TINYOBJLOADER_IMPLEMENTATION ///< This MUST be defined before importing tiny_obj_loader.h
#include <tiny_obj_loader.h>
#include <robin_hood.h>
#include <HFExceptions.h>
#include <iostream>
#include <vector>
#include <filesystem>

using std::vector;
using std::array;
using std::string;

namespace HF::Geometry {

	static robin_hood::unordered_map<string, string> test_model_paths{
		{"teapot", "teapot.obj"},
		{"plane", "plane.obj" },
		{"big teapot", "big_teapot.obj" },
		{"energy blob", "energy_blob.obj" },
		{"sibenik", "sibenik.obj" },
	};
	
	vector<MeshInfo> LoadMeshObjects(std::string path, GROUP_METHOD gm, bool change_coords)
	{
		// First, attempt to load the obj
		tinyobj::ObjReader objloader;

		// See if the filepath exists at all
		if (!std::filesystem::exists(path)) {
			std::cerr << " No file exists at " << path << std::endl;
			throw HF::Exceptions::FileNotFound();
		}

		// Load the OBJ using tinyobj.
		objloader.ParseFromFile(path);

		// Throw if the OBJ isn't valid
		if (!objloader.Valid() || objloader.GetShapes().size() == 0)
		{
			std::cerr << " The given file did not produce a valid mesh " << std::endl;
			throw HF::Exceptions::InvalidOBJ();
		}

		// get the materials, attributes, and shapes
		vector<MeshInfo> MI; // Vector of meshinf
		auto mats = objloader.GetMaterials(); // Materials of the mesh at path
		tinyobj::attrib_t attributes = objloader.GetAttrib(); // Attributes of the mesh at path
		vector<tinyobj::shape_t> shapes = objloader.GetShapes(); // Submeshes of the mesh at path
		const vector<tinyobj::real_t>& verts = attributes.vertices; // Vertices of the mesh at path.

		// This will behave differently depending on the group_type being used.
		switch (gm) {
		case GROUP_METHOD::ONLY_FILE: {
			// A single mesh will just need to have its index arrays combined
			int id = 0;
			std::string name = "EntireFile";
			const vector<float>& vertexes = static_cast<vector<float>>(verts);
			vector<int> indices;

			// Count total indexes
			int index_count = 0;
			for (auto& shape : shapes)
				index_count += shape.mesh.indices.size();

			// Create a giant vector based on those indices
			vector<int> index_array(index_count);

			// Copy index arrays from every shape in shapes into the single index array we defined previously.
			int last_index = 0;
			for (auto& shape : shapes)
				for (auto& this_shape_index : shape.mesh.indices)
					index_array[last_index++] = this_shape_index.vertex_index;

			// Add this mesh to the output_array
			MI.emplace_back(MeshInfo(vertexes, index_array, 0, name));

			// Change from Y-up to Z-up if specified.
			if (change_coords) MI[0].ConvertToRhinoCoordinates();
			break;
		}

		case GROUP_METHOD::BY_GROUP: {
			// Each group represents a different mesh.

			// Allocate a space for every shape in shapes.
			// Note: Shapes are OBJ groups.
			MI.resize(shapes.size());
			vector<array<float, 3>> current_vertices;

			// Iterate through every shape in shapes.
			for (int k = 0; k < shapes.size(); k++) {
				auto& shape = shapes[k];
				// Set MeshInfo parameters for shape
				std::string name = shape.name;
				int id = k;
				auto& indicies = shape.mesh.indices;

				// Add vertices to shape's list of vertices.
				for (int i = 0; i < indicies.size(); i++) {
					tinyobj::index_t idx = indicies[i];

					int vertex_index = 3 * idx.vertex_index;
					float x = verts[vertex_index];
					float y = verts[1 + vertex_index];
					float z = verts[2 + vertex_index];

					current_vertices.emplace_back(array<float, 3>{x, y, z});
				}

				// Finally add this mesh to the list of meshes, then rotate it to Z-up if specified.
				MI[k] = MeshInfo(current_vertices, id, name);
				if (change_coords) MI[k].ConvertToRhinoCoordinates();
			}
			break;
		}
		case GROUP_METHOD::BY_MATERIAL: {
			// If there are no materials, just put every vert into the same mesh
			if (mats.empty()) {
				std::cerr << "[C++] No materials found in model " << path << ". Grouping by obj group";
				return LoadMeshObjects(path, BY_GROUP);
			}

			// Create a vector of [mesh_id:"material"] for every unique material
			MI.resize(mats.size());
			vector<vector< array<float, 3 >>> verts_by_mat_id(mats.size());

			// Create and fill a vector of names for each material.
			vector<std::string> names;
			for (int i = 0; i < mats.size(); i++)
				names.emplace_back(path + "/" + mats[i].name);

			// Loop through every shape to fill the vector
			for (int k = 0; k < shapes.size(); k++) {
				auto& shape = shapes[k];
				vector<int>& mat_ids = shape.mesh.material_ids;
				vector<tinyobj::index_t>& indicies = shape.mesh.indices;

				// Iterate through every index in indices.
				for (int i = 0; i < indicies.size(); i++) {
					tinyobj::index_t idx = indicies[i];

					int vertex_index = 3 * idx.vertex_index;

					float x = verts[vertex_index];
					float y = verts[1 + vertex_index];
					float z = verts[2 + vertex_index];

					// Get the index of this face (equal to i/3) since every 3 indices is a face
					int face = static_cast<int>(floor(i / 3));

					// Get the material index of face.
					if (mats.size() > 0 && mat_ids.size() > face) {
						int mat_index = mat_ids[face];

						verts_by_mat_id[mat_index].emplace_back(array<float, 3>{x, y, z});
					}
				}
			}

			// Reassign meshids and cull materials with no geometry.
			for (int i = 0; i < names.size(); i++) {
				auto& mesh = verts_by_mat_id[i];
				if (mesh.empty()) continue; // Ignore unused materials

				// add to list;
				MI.push_back(MeshInfo(mesh, i, names[i]));
				if (change_coords) MI[i].ConvertToRhinoCoordinates();
			}
			break;
		}

		// Throw because we were given an invalid group mode.
		default:
			std::cerr << "Mesh group mode" << gm << " doesn't exist!" << std::endl;
			throw std::exception();
			break;
		}

		// Throw if the obj file didn't contain any geometry.
		if (MI.size() == 0) {
			throw HF::Exceptions::InvalidOBJ();
		}
		return MI;
	}

	vector<array<float, 3>> LoadRawVertices(std::string path)
	{
		// First, attempt to load the obj
		tinyobj::ObjReader objloader;
		objloader.ParseFromFile(path);
		if (!objloader.Valid()) throw HF::Exceptions::InvalidOBJ();

		// get the materials, attributes, and shapes
		auto mats = objloader.GetMaterials();
		tinyobj::attrib_t attributes = objloader.GetAttrib();
		vector<tinyobj::shape_t> shapes = objloader.GetShapes();
		vector<tinyobj::real_t>& verts = attributes.vertices;

		// Create out_verts array
		vector<array<float, 3>> out_verts;

		// iterate through every shape in shapes
		for (auto& shape : shapes) {
			auto& indicies = shape.mesh.indices;

			// Copy into a new array.
			for (int i = 0; i < indicies.size(); i++) {
				tinyobj::index_t idx = indicies[i];

				int vertex_index = 3 * idx.vertex_index;

				float x = verts[vertex_index];
				float y = verts[1 + vertex_index];
				float z = verts[2 + vertex_index];
				out_verts.emplace_back(array<float, 3>{x, y, z});
			}
		}

		return out_verts;
	}

	std::string GetTestOBJPath(std::string key)
	{
		return test_model_paths.at(key);
	}

	vector<MeshInfo> LoadMeshObjects(vector<std::string>& path, GROUP_METHOD gm, bool change_coords)
	{
		// Create a vector of vectors and gather all individual results
		vector<vector<MeshInfo>> MeshObjects(path.size());
		for (int i = 0; i < path.size(); i++)
			MeshObjects[i] = LoadMeshObjects(path[i], gm, change_coords);

		// Compress all vectors into a single mesh and reassign meshid.
		vector<MeshInfo> MI;
		int lid = 0;
		for (int i = 0; i < MeshObjects.size(); i++) {
			for (auto& mi : MeshObjects[i]) {
				MI.push_back(mi);
				MI.back().SetMeshID(lid++);
			}
		}

		return MI;
	}
}