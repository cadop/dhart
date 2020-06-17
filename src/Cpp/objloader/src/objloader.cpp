///
///	\file		objloader.cpp
///	\brief		Source file for functions in the HF::Geometry namespace
///
///	\author		TBA
///	\date		16 Jun 2020
///
#include <objloader.h>
#include <Dense>
#include <meshinfo.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <robin_hood.h>
#include <HFExceptions.h>
#include <iostream>
#include <vector>
#include <filesystem>

namespace HF {
	namespace Geometry {
		/// <summary>
		/// Create an indexed array of vertices from a tinyobj::shape and list of verts
		/// </summary>
		/// <param name="shape">Shape to index</param>
		/// <param name="verts">Vertex array from tinyobjloader</param>
		/// <param name="out_indices">Output array for indexes</param>
		/// <param name="out_vertexes">Output array for vertices</param>
		void IndexShape(const tinyobj::shape_t& shape, const std::vector<tinyobj::real_t>& verts, std::vector<int> out_indices, std::vector<float> out_vertexes) {
			// Due to the fact that verts is indexed for the entire object
			// instead of per shape, we need to remap the ids in verts to our own ids
			auto& indices = shape.mesh.indices;
			out_indices.reserve(indices.size());

			robin_hood::unordered_map<int, int> remap;  //<their_id, our_id>
			int last_id = 0; // Increment this with each new vertex seen

			// Create array of indices
			for (auto& index : indices)
			{
				const int their_id = index.vertex_index;
				int our_id = -1;

				// Get Id from the map or assign it if needed
				if (remap.count(their_id) != 0)  our_id = remap[their_id];
				else {
					our_id = last_id++;
					remap[their_id] = our_id;
				}

				// Push it back into our output index array
				out_indices.push_back(our_id);
			}

			// Now create our array of vertexes based on the array of ids
			const int unique_vertex_count = remap.size();
			out_vertexes.resize(unique_vertex_count * 3);
			for (auto it = remap.begin(); it != remap.end(); ++it) {
				auto their_id = it->first; auto our_id = it->second;

				const auto their_index = their_id * 3;
				const auto our_index = our_id * 3;

				out_vertexes[our_index] = verts[their_index];
				out_vertexes[our_index + 1] = verts[their_index + 1];
				out_vertexes[our_index + 2] = verts[their_index + 2];
			}
		}
		std::vector<MeshInfo> LoadMeshObjects(std::string path, GROUP_METHOD gm, bool change_coords)
		{
			// First, attempt to load the obj
			tinyobj::ObjReader objloader;

			// See if the filepath exists at all 
			if (!std::filesystem::exists(path)) {
				std::cerr << " No file exists at " << path << std::endl;
				throw HF::Exceptions::FileNotFound();
			}

			objloader.ParseFromFile(path);

			if (!objloader.Valid() || objloader.GetShapes().size() == 0)
			{
				std::cerr << " The given file did not produce a valid mesh " << std::endl;
				throw HF::Exceptions::InvalidOBJ();
			}

			std::vector<MeshInfo> MI;

			// get the materials, attributes, and shapes
			auto mats = objloader.GetMaterials();
			tinyobj::attrib_t attributes = objloader.GetAttrib();
			std::vector<tinyobj::shape_t> shapes = objloader.GetShapes();
			const std::vector<tinyobj::real_t>& verts = attributes.vertices;

			// This will behave differently depending on the mesh being used
			switch (gm) {
			case GROUP_METHOD::ONLY_FILE: {
				// A single mesh will just need to have its index arrays
				// combined
				int id = 0;
				std::string name = "EntireFile";
				const std::vector<float>& vertexes = static_cast<std::vector<float>>(verts);
				std::vector<int> indices;

				// Count total indexes
				int index_count = 0;
				for (auto& shape : shapes)
					index_count += shape.mesh.indices.size();

				// Create a giant vector based on those indices
				std::vector<int> index_array(index_count);

				//Move Index arrays to our array one by one
				int last_index = 0;
				for (auto& shape : shapes)
					for (auto& this_shape_index : shape.mesh.indices)
						index_array[last_index++] = this_shape_index.vertex_index;//std::move(this_shape_indices[i].vertex_index);

				MI.emplace_back(MeshInfo(vertexes, index_array, 0, name));
				if (change_coords) MI[0].ConvertToRhinoCoordinates();
				break;
			}

			case GROUP_METHOD::BY_GROUP: {
				// Each group represents a different mesh.
				MI.resize(shapes.size());
				std::vector<std::array<float, 3>> current_vertices;
				for (int k = 0; k < shapes.size(); k++) {
					auto& shape = shapes[k];

					// Set parameters for shape
					std::string name = path + shape.name;
					int id = k;
					auto& indicies = shape.mesh.indices;

					// Add vertices to this list
					for (int i = 0; i < indicies.size(); i++) {
						tinyobj::index_t idx = indicies[i];

						int vertex_index = 3 * idx.vertex_index;
						float x = verts[vertex_index];
						float y = verts[1 + vertex_index];
						float z = verts[2 + vertex_index];

						current_vertices.emplace_back(std::array<float, 3>{x, y, z});
					}
					// Finally add this mesh to the list and change it to rhino coords
					MI[k] = MeshInfo(current_vertices, id, name);
					if (change_coords) MI[k].ConvertToRhinoCoordinates();
				}
				break;
			}
			case GROUP_METHOD::BY_MATERIAL: {
				// If there are no materials, just put every vert into the same mesh
				if (mats.empty()) {
					std::cout << "[C++] No materials found in model " << path << ". Grouping by obj group";
					return LoadMeshObjects(path, BY_GROUP);
				}

				// Create a vector of [mesh_id:"material"] for every unique material
				MI.resize(mats.size());
				std::vector<std::string> names;
				std::vector<std::vector< std::array<float, 3 >>> verts_by_mat_id(mats.size());
				for (int i = 0; i < mats.size(); i++)
					names.emplace_back(path + "/" + mats[i].name);

				// Loop through every shape to fill the vector
				for (int k = 0; k < shapes.size(); k++) {
					auto& shape = shapes[k];
					std::vector<int>& mat_ids = shape.mesh.material_ids;
					std::vector<tinyobj::index_t>& indicies = shape.mesh.indices;

					for (int i = 0; i < indicies.size(); i++) {
						tinyobj::index_t idx = indicies[i];

						int vertex_index = 3 * idx.vertex_index;

						float x = verts[vertex_index];
						float y = verts[1 + vertex_index];
						float z = verts[2 + vertex_index];

						// Materials are indexed by face, not by vert
						int face = static_cast<int>(floor(i / 3));
						if (mats.size() > 0 && mat_ids.size() > face) {
							int mat_index = mat_ids[face];

							verts_by_mat_id[mat_index].emplace_back(std::array<float, 3>{x, y, z});
						}
					}
				}

				// Add and convert meshes to their own mesh IDs
				for (int i = 0; i < names.size(); i++) {
					auto& mesh = verts_by_mat_id[i];
					if (mesh.empty()) continue; // Ignore unused materials

					MI.push_back(MeshInfo(mesh, i, names[i]));
					if (change_coords) MI[i].ConvertToRhinoCoordinates();
				}
				break;
			}
			default:
				std::cerr << "Mesh group mode" << gm << " doesn't exist!" << std::endl;
				throw std::exception();
				break;
			}
			
			if (MI.size() == 0) {
				throw HF::Exceptions::InvalidOBJ();
			}
			return MI;
		}

		std::vector<std::array<float, 3>> LoadRawVertices(std::string path)
		{
			// First, attempt to load the obj
			tinyobj::ObjReader objloader;
			objloader.ParseFromFile(path);
			if (!objloader.Valid()) throw HF::Exceptions::InvalidOBJ();

			// get the materials, attributes, and shapes
			auto mats = objloader.GetMaterials();
			tinyobj::attrib_t attributes = objloader.GetAttrib();
			std::vector<tinyobj::shape_t> shapes = objloader.GetShapes();
			std::vector<tinyobj::real_t>& verts = attributes.vertices;

			std::vector<std::array<float, 3>> out_verts;

			for (auto& shape : shapes) {
				auto& indicies = shape.mesh.indices;
				for (int i = 0; i < indicies.size(); i++) {
					tinyobj::index_t idx = indicies[i];

					int vertex_index = 3 * idx.vertex_index;

					float x = verts[vertex_index];
					float y = verts[1 + vertex_index];
					float z = verts[2 + vertex_index];
					out_verts.emplace_back(std::array<float, 3>{x, y, z});
				}
			}

			return out_verts;
		}
		std::vector<MeshInfo> LoadMeshObjects(std::vector<std::string>& path, GROUP_METHOD gm, bool change_coords)
		{
			// create a vector of vectors and gather all individual results
			std::vector<std::vector<MeshInfo>> MeshObjects(path.size());
			for (int i = 0; i < path.size(); i++)
				MeshObjects[i] = LoadMeshObjects(path[i], gm, change_coords);

			// Reassign IDs and place into single list
			std::vector<MeshInfo> MI;
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
}
