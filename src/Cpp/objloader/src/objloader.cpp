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


// [nanoRT]
namespace HF::nanoGeom {

	bool LoadObj(Mesh& mesh, const char* filename) {
		// Need to have mesh.vertices, mesh.faces, mesh.num_faces

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::string warn;
		std::string err;

		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename);

		// Check for errors in the loading
		if (!err.empty()) {
			std::cerr << " Loading: " << err << std::endl;
		}
		if (!ret) {
			exit(1);
		}

		// Get the number of faces (likely number of triangles)
		size_t num_faces = 0;
		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++)
		{
			// Increment the number of faces by getting the size of the indices and dividing by 3 (number of verts per face)
			// Must be triangles for this calculation to make sense (I think)
			num_faces += shapes[s].mesh.indices.size() / 3;
		}

		// Set the number of vertices by finding the total size of the vertex array and dividing by 3 (number of axis in point: x,y,z)
		size_t num_vertices = attrib.vertices.size() / 3;
		mesh.num_faces = num_faces;
		mesh.num_vertices = num_vertices;
		// Set a new array of doubles to the size of the number of vertices times 3 (number of axis in point)
		mesh.vertices = new double[num_vertices * 3];
		// Set a new array of doubles to the size of the number of faces times 3 (number of verts per face)
		// While this is called faces, its really the indices
		mesh.faces = new unsigned int[num_faces * 3];

		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++)
		{
			// Loop over faces(polygon)
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
			{
				int fv = shapes[s].mesh.num_face_vertices[f];

				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++)
				{
					// extract the index of the start of the mesh indices
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					// Assign the xyz values from the flat array
					tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
					tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
					tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];

					// set vertices to the mesh data object
					// This could probably all be done in one step with the above 3 lines
					mesh.vertices[3 * idx.vertex_index + 0] = vx;
					mesh.vertices[3 * idx.vertex_index + 1] = vy;
					mesh.vertices[3 * idx.vertex_index + 2] = vz;
				}
				index_offset += fv;
			}

			// Loop through the mesh indices and assign each vertices index to the face index
			for (size_t vi = 0; vi < shapes[s].mesh.indices.size(); vi++)
			{
				// This should make it clearer that this really is the mesh indices despite the name "faces"
				mesh.faces[vi] = shapes[s].mesh.indices[vi].vertex_index;
			}
		}
		return true;
	}

}
// end [nanoRT]

namespace HF::Geometry {

	static robin_hood::unordered_map<string, string> test_model_paths{
		{"teapot", "teapot.obj"},
		{"plane", "plane.obj" },
		{"big teapot", "big_teapot.obj" },
		{"energy blob", "energy_blob.obj" },
		{"sibenik", "sibenik.obj" },
	};
	
	vector<MeshInfo> LoadMeshObjects(std::string path, GROUP_METHOD gm, bool change_coords, int scale)
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
			
			// Must be included in the other method types. It's not clear why the float conversion 
			// wasn't done above with the attributes.vertices
			auto vert_array = static_cast<vector<float>>(verts);
			auto vert_scaled = vert_array;
			// Multiply each element of array by the user defined scale value (default of 1 which does not change vertex)
			std::transform(vert_array.begin(), vert_array.end(), vert_scaled.begin(),[&](float i) { return i * scale; });
			// assign the float vector to the scaled method
			const vector<float>& vertexes = vert_scaled;


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
				std::string name = path + shape.name;
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
				std::cout << "[C++] No materials found in model " << path << ". Grouping by obj group";
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

	vector<MeshInfo> LoadMeshObjects(vector<std::string>& path, GROUP_METHOD gm, bool change_coords, int scale)
	{
		// Create a vector of vectors and gather all individual results
		vector<vector<MeshInfo>> MeshObjects(path.size());
		for (int i = 0; i < path.size(); i++)
			MeshObjects[i] = LoadMeshObjects(path[i], gm, change_coords, scale);

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