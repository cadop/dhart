#include <gtest/gtest.h>
#include <objloader.h>
#include <meshinfo.h>
#include <HFExceptions.h>
#include <string>

#include "objloader_C.h"

#define eigen_plain_assert

const std::string obj_directiory = "../../Models/";
void PrintArray(std::array<float, 3> in_array) {
	std::cerr << "(" << in_array[0] << "," << in_array[1] << "," << in_array[2] << ")";
}


inline float arrayDist(const std::array<float, 3> from, const std::array<float, 3>& to) {
	return sqrtf(powf(from[0] - to[0], 2) + powf(from[1] - to[1], 2) + powf(from[2] - to[2], 2));
}

void CompareMeshInfo(const HF::Geometry::MeshInfo& MI1, const HF::Geometry::MeshInfo& MI2, std::string label1 = "MI1", std::string label2 = "MI2") {
	ASSERT_EQ(MI1.NumVerts(), MI2.NumVerts());
	int fail_count = 0;
	for (int i = 0; i < MI1.NumVerts(); i++) {
		auto this_array = MI1[i];
		auto that_array = MI2[i];

		auto dist = arrayDist(this_array, that_array);
		if (!(dist < 0.001f)) {
			fail_count++;
			if (fail_count > 5) {
				printf("Failure count exceeded 5");
				ASSERT_TRUE(false);
			}

			std::cerr << "Index: "  << i << std::endl << "Distance: " << dist << std::endl << label1;
			PrintArray(this_array);
			std::cerr << std::endl << label2;
			PrintArray(that_array);
			std::cerr << std::endl;
			EXPECT_LE(dist, 0.001f);
		}
	}
}

void CompareVertArrays(const std::vector<std::array<float, 3>>& vert_array1, const std::vector<std::array<float, 3>> & vert_array2, std::string label1 = "MI1", std::string label2 = "MI2") {
	int fail_count = 0;

	ASSERT_EQ(vert_array1.size(), vert_array2.size());
	for (int i = 0; i < vert_array1.size(); i++) {
		auto this_array = vert_array1[i];
		auto that_array = vert_array2[i];

		auto dist = arrayDist(this_array, that_array);
		if (!(dist < 0.001f)) {
			fail_count++;
			if (fail_count > 5)
				ASSERT_TRUE(false);

			std::cerr << "Index: " << i << std::endl << "Distance: " << dist << std::endl << label1;
			PrintArray(this_array);
			std::cerr << std::endl << label2;
			PrintArray(that_array);
			std::cerr << std::endl;
			EXPECT_LE(dist, 0.001f);
		}
	}
}
TEST(_OBJLoader, ThrowMissingFileOnMissingMesh) {
	auto paths = std::vector<std::string>{"ThisMeshDoesn'tExist" };
	ASSERT_THROW(HF::Geometry::LoadMeshObjects(paths, HF::Geometry::BY_GROUP), HF::Exceptions::FileNotFound);
}

TEST(_OBJLoader, VertexCountMatchesModel) {
	auto paths = std::vector<std::string>{ "big_teapot.obj" }; // This is located in the folder where the EXE is
	auto MI = HF::Geometry::LoadRawVertices(paths[0]);

	ASSERT_EQ(MI.size(), 18960);
}

TEST(_OBJLoader, By_File) {
	auto paths = std::vector<std::string>{ "teapot.obj" }; // This is located in the folder where the EXE is
	auto MI = HF::Geometry::LoadMeshObjects(paths, HF::Geometry::ONLY_FILE);

	EXPECT_TRUE(MI[0].NumTris() > 0);
	EXPECT_TRUE(MI[0].NumVerts() > 0);
	EXPECT_TRUE(MI[0].GetMeshID() == 0);
}

TEST(_OBJLoader, By_Material) {
	auto paths = std::vector<std::string>{ "teapot.obj" }; // This is located in the folder where the EXE is
	auto MI = HF::Geometry::LoadMeshObjects(paths, HF::Geometry::ONLY_FILE);

	EXPECT_TRUE(MI[0].NumTris() > 0);
	EXPECT_TRUE(MI[0].NumVerts() > 0);
	EXPECT_TRUE(MI[0].GetMeshID() == 0);
}

TEST(_MeshInfo, ManualRotationEqualsPreset) {
	auto test_paths = std::vector<std::string>{ "teapot.obj" }; // This is located in the folder where the EXE is
	
	auto test_MI = HF::Geometry::LoadMeshObjects(test_paths, HF::Geometry::ONLY_FILE, false)[0];
	auto base_MI = HF::Geometry::LoadMeshObjects(test_paths, HF::Geometry::ONLY_FILE, false)[0];

	test_MI.ConvertToRhinoCoordinates();
	base_MI.PerformRotation(90, 0, 0);
	CompareMeshInfo(test_MI, base_MI, "RotatedFromOBJ", "RhinoStandard");
}

TEST(_MeshInfo, RotationToRhinoIsAccurate) {
	auto test_paths = std::vector<std::string>{ "teapot.obj" }; // This is located in the folder where the EXE is
	auto base_paths = std::vector<std::string>{ "corrected_teapot.obj" }; // This is located in the folder where the EXE is
	auto test_MI = HF::Geometry::LoadMeshObjects(test_paths, HF::Geometry::ONLY_FILE, false)[0];
	auto base_MI = HF::Geometry::LoadMeshObjects(base_paths, HF::Geometry::ONLY_FILE, false)[0];

	test_MI.ConvertToRhinoCoordinates();
	CompareMeshInfo(test_MI, base_MI, "RotatedFromOBJ", "RhinoStandard");
}

TEST(_MeshInfo, CopyConstructorWorks) {
	auto test_paths = std::vector<std::string>{ "big_teapot.obj" }; // This is located in the folder where the EXE is
	auto test_MI = HF::Geometry::LoadMeshObjects(test_paths, HF::Geometry::ONLY_FILE)[0];

	auto copy_MI = test_MI;
	auto copy2_MI = copy_MI;
	copy2_MI.ConvertToRhinoCoordinates();

	CompareMeshInfo(test_MI, copy_MI, "base", "copied");
}

TEST(_MeshInfo, CreationIsDeterministic) {
	auto test_paths = std::vector<std::string>{ "big_teapot.obj" }; // This is located in the folder where the EXE is
	auto load1_MI = HF::Geometry::LoadMeshObjects(test_paths, HF::Geometry::ONLY_FILE, false)[0];
	auto load2_MI = HF::Geometry::LoadMeshObjects(test_paths, HF::Geometry::ONLY_FILE, false)[0];

	CompareMeshInfo(load1_MI, load2_MI, "first", "second");
}

TEST(_MeshInfo, CanDisableRotationOnLoad) {
	auto test_paths = std::vector<std::string>{ "big_teapot.obj" }; // This is located in the folder where the EXE is
	auto load1_MI = HF::Geometry::LoadMeshObjects(test_paths, HF::Geometry::ONLY_FILE)[0];
	auto load2_MI = HF::Geometry::LoadMeshObjects(test_paths, HF::Geometry::ONLY_FILE, false)[0];

	ASSERT_FALSE(load1_MI == load2_MI);
}
TEST(_MeshInfo, RotationIsDeterministic) {
	auto test_paths = std::vector<std::string>{ "big_teapot.obj" }; // This is located in the folder where the EXE is
	auto load1_MI = HF::Geometry::LoadMeshObjects(test_paths, HF::Geometry::ONLY_FILE)[0];
	auto load2_MI = HF::Geometry::LoadMeshObjects(test_paths, HF::Geometry::ONLY_FILE)[0];


	CompareMeshInfo(load1_MI, load2_MI, "first", "second");
}


TEST(_MeshInfo, RotationIsReversable) {
	auto obj_coord_paths = std::vector<std::string>{ "teapot.obj" }; // This is located in the folder where the EXE is
	auto rhino_coord_paths = std::vector<std::string>{ "teapot.obj" }; // This is located in the folder where the EXE is
	auto obj_coord_info = HF::Geometry::LoadMeshObjects(obj_coord_paths, HF::Geometry::ONLY_FILE, false)[0];
	auto rhino_coord_info = HF::Geometry::LoadMeshObjects(obj_coord_paths, HF::Geometry::ONLY_FILE, false)[0];

	obj_coord_info.ConvertToOBJCoordinates();
	obj_coord_info.ConvertToRhinoCoordinates();

	CompareMeshInfo(obj_coord_info, rhino_coord_info, "Rotated", "Unmodified");
}

TEST(_MeshInfo, CanReproduceInputArray) {
	auto obj_coord_paths = std::vector<std::string>{ "corrected_teapot.obj" }; // This is located in the folder where the EXE is
	auto raw_verts = HF::Geometry::LoadRawVertices(obj_coord_paths[0]);
	auto MI = HF::Geometry::LoadMeshObjects(obj_coord_paths[0], HF::Geometry::GROUP_METHOD::ONLY_FILE, false);
	
	CompareVertArrays(raw_verts, MI[0].GetVertsAsArrays());
}

///
///	The following are tests for the code samples for HF::Geometry
///

/// start objloader.h
TEST(_meshInfo, LoadMeshObjectsOne) {
	// be sure to #include "objloader.h", and #include <vector>

	// Relative path begins where EXE file is located
	// If file_path is not a path to a valid OBJ file, HF::Exceptions::FileNotFound is thrown
	std::string file_path = "big_teapot.obj";

	// meshvec is a vector of meshinfo from file_path
	std::vector<HF::Geometry::MeshInfo> meshvec = HF::Geometry::LoadMeshObjects(file_path, HF::Geometry::GROUP_METHOD::ONLY_FILE, false);

	// Retrieve the MeshInfo
	HF::Geometry::MeshInfo info = meshvec[0];
}

TEST(_meshInfo, LoadMeshObjectsMany) {
	// be sure to #include "objloader.h", and #include <vector>

	// Prepare the file paths 
	const auto PATH_0 = "big_teapot.obj";
	const auto PATH_1 = "plane.obj";
	const auto PATH_2 = "sibenik.obj";

	// PATH_0, PATH_1, and PATH_2 each represent a path to an OBJ file.
	// Note that if any path in pathvec is invalid, HF::Exceptions::FileNotFound will be thrown
	// when LoadMeshObjects is called 
	std::vector<std::string> pathvec = { PATH_0, PATH_1, PATH_2 };

	// The overload for LoadMeshObjects is called for each member (which is an OBJ file path) in pathvec,
	// then IDs are reassigned for each MeshInfo object within the std::vector<MeshInfo> that is returned
	std::vector<HF::Geometry::MeshInfo> meshvec = HF::Geometry::LoadMeshObjects(pathvec, HF::Geometry::GROUP_METHOD::ONLY_FILE, false);

	std::cout << "Total loaded: " << meshvec.size() << std::endl;

	// Print IDs of all mesh info
	for (auto mesh_info : meshvec) {
		std::cout << "Mesh ID: " << mesh_info.GetMeshID() << std::endl;
	}
}

TEST(_meshInfo, LoadRawVertices) {
	// be sure to #include "objloader.h", and #include <vector>

	// Note that filepath must lead to a valid OBJ file, or
	// HF::Exceptions::InvalidOBJ will be thrown when Geometry::LoadMeshObjects is called
	std::string filepath = "plane.obj";

	// Vertices now contains the raw vertices loaded from the OBJ file specified at filepath
	std::vector<std::array<float, 3>> vertices = HF::Geometry::LoadRawVertices(filepath);
	
	// Display the vertices from filepath
	std::cout << "Vertices from " << filepath << ": " << std::endl;
	for (auto vertex : vertices) {
		std::cout << "(" << vertex[0] << ", " << vertex[1] << ", " << vertex[2] << ")" << std::endl;
	}
}
/// end objloader.h

TEST(_meshInfo, ConstructorDefault) {
	// be sure to #include "meshinfo.h"

	HF::Geometry::MeshInfo mesh;	// meshid == 0; verts is given 3 rows, 0 cols; name == "INVALID"
}

TEST(_meshInfo, ConstructorParamCoordsAsArray) {
	// be sure to #include "meshinfo.h", and #include <vector>

	// Prepare the vertices
	std::array<float, 3> vertex_0 = { 34.1, 63.9, 16.5 };
	std::array<float, 3> vertex_1 = { 23.5, 85.7, 45.2 };
	std::array<float, 3> vertex_2 = { 12.0, 24.6, 99.4 };

	// Create an array of vertices
	std::vector<std::array<float, 3>> vertices{ vertex_0, vertex_1, vertex_2 };

	// This mesh contains a triangle (due to having 3 vertices), id == 3451, name == "My Mesh"
	// Note that vertices is passed by reference.
	// Also, passing a std::vector<std::array<float, 3>> with (size < 1) will cause
	// HF::Exceptions::InvalidOBJ to be thrown.
	HF::Geometry::MeshInfo mesh(vertices, 3451, "My Mesh");

	// Display the vertices for mesh
	std::cout << "Vertices in mesh with ID " << mesh.GetMeshID() << ": " << std::endl;
	for (auto vertex : mesh.GetVertsAsArrays()) {
		std::cout << "(" << vertex[0] << ", " << vertex[1] << ", " << vertex[2] << ")" << std::endl;
	}
}

TEST(_meshInfo, ConstructorParamCoordsAsFloat) {
	// be sure to #include "meshinfo.h", and #include <vector>

	// Prepare the vertices. Every three floats represents one vertex.
	std::vector<float> vertices = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
	                            //    0					1				2

	// If indices.size() == 3, this means that vertices.size() == 9,
	// and each member of indices represents a std::array<float, 3>
	std::vector<int> indices = { 0, 1, 2 };

	// Note that vertices and indices are passed by reference.
	// Also, passing a std::vector<float> with a size that is not a multiple of 3,
	// or passing a std::vector<int> with a size that is not a multiple of 3 --
	// will cause HF::Exceptions::InvalidOBJ to be thrown.
	// Also, indices.size() == (vertices.size() / 3), because all members of indices
	// must correspond to the index of the first member representing the initial coordinate within a vertex.
	HF::Geometry::MeshInfo mesh(vertices, indices, 5901, "This Mesh");

	// Display the vertices for mesh
	std::cout << "Vertices in mesh with ID " << mesh.GetMeshID() << ": " << std::endl;
	for (auto vertex : mesh.GetVertsAsArrays()) {
		std::cout << "(" << vertex[0] << ", " << vertex[1] << ", " << vertex[2] << ")" << std::endl;
	}
}

TEST(_meshInfo, AddVertsAsArray) {
	// be sure to #include "meshinfo.h", and #include <vector>

	// Prepare the vertices
	std::array<float, 3> vertex_0 = { 34.1, 63.9, 16.5 };
	std::array<float, 3> vertex_1 = { 23.5, 85.7, 45.2 };
	std::array<float, 3> vertex_2 = { 12.0, 24.6, 99.4 };

	// Create a vector of vertices
	std::vector<std::array<float, 3>> vertices{ vertex_0, vertex_1, vertex_2 };

	// Create the MeshInfo instance - this example uses the no-arg constructor
	HF::Geometry::MeshInfo mesh;	// meshid == 0; verts is given 3 rows, 0 cols; name == "INVALID"

	// Append the vertices to the mesh
	mesh.AddVerts(vertices);

	std::cout << "size: " << mesh.getRawVertices().size() << std::endl;

	// Display the vertices for mesh
	std::cout << "Vertices in mesh with ID " << mesh.GetMeshID() << ": " << std::endl;
	for (auto vertex : mesh.GetVertsAsArrays()) {
		std::cout << "(" << vertex[0] << ", " << vertex[1] << ", " << vertex[2] << ")" << std::endl;
	}
}

TEST(_meshInfo, AddVertsAsFloatAndIndices) {
	/*
	// be sure to #include "meshinfo.h", and #include <vector>

	// TODO: needs implementation

	// Prepare the vertices. Every three floats represents one vertex.
	std::vector<float> vertices = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
												//    0					1				2

	// If indices.size() == 3, this means that vertices.size() == 9,
	// and each member of indices represents a std::array<float, 3>
	std::vector<int> indices = { 0, 1, 2 };

	// Create the MeshInfo instance - this example uses the no-arg constructor
	HF::Geometry::MeshInfo mesh;	// meshid == 0; verts is given 3 rows, 0 cols; name == "INVALID"

	// Append the vertices to the mesh.
	mesh.AddVerts(vertices, indices);
	*/
}

TEST(_meshInfo, NumVerts) {
	// be sure to #include "meshinfo.h", and #include <vector>

	// Prepare the vertices
	std::array<float, 3> vertex_0 = { 34.1, 63.9, 16.5 };
	std::array<float, 3> vertex_1 = { 23.5, 85.7, 45.2 };
	std::array<float, 3> vertex_2 = { 12.0, 24.6, 99.4 };

	// Create an array of vertices
	std::vector<std::array<float, 3>> vertices{ vertex_0, vertex_1, vertex_2 };

	// Create the MeshInfo
	HF::Geometry::MeshInfo mesh(vertices, 3451, "My Mesh");

	int vert_count = 0;
	if ((vert_count = mesh.NumVerts()) == 0) {
		std::cout << "This mesh has no vertices." << std::endl;
	} else {
		std::cout << "Vertex count: " << vert_count << std::endl;
	}

	// Output is: 'Vertex count: 3'
}

TEST(_meshInfo, NumTris) {
	// be sure to #include "meshinfo.h", and #include <vector>

	// Prepare the vertices
	std::array<float, 3> vertex_0 = { 34.1, 63.9, 16.5 };
	std::array<float, 3> vertex_1 = { 23.5, 85.7, 45.2 };
	std::array<float, 3> vertex_2 = { 12.0, 24.6, 99.4 };

	// Create an array of vertices
	std::vector<std::array<float, 3>> vertices{ vertex_0, vertex_1, vertex_2 };

	// Create the MeshInfo
	HF::Geometry::MeshInfo mesh(vertices, 3451, "My Mesh");

	int tri_count = 0;
	if ((tri_count = mesh.NumTris()) == 0) {
		std::cout << "This mesh has no triangles." << std::endl;
	} else {
		std::cout << "Triangle count: " << tri_count << std::endl;
	}

	// Output is: 'Triangle count: 1'
	// Note that (mesh.NumVerts() / 3 && mesh.NumTris()) will always be true,
	// since three vertices compose a single triangle.
}

TEST(_meshInfo, ConvertToRhinoCoordinates) {
	// be sure to #include "meshinfo.h", and #include <vector>

	// Prepare the vertices
	std::array<float, 3> vertex_0 = { 34.1, 63.9, 16.5 };
	std::array<float, 3> vertex_1 = { 23.5, 85.7, 45.2 };
	std::array<float, 3> vertex_2 = { 12.0, 24.6, 99.4 };

	// Create an array of vertices
	std::vector<std::array<float, 3>> vertices{ vertex_0, vertex_1, vertex_2 };

	// Create the MeshInfo
	HF::Geometry::MeshInfo mesh(vertices, 3451, "My Mesh");

	// Convert the coordinates
	mesh.ConvertToRhinoCoordinates();

	// Note: If vertices contain NaN or +/- infinity values, std::exception is thrown

	// Display the vertices for mesh
	std::cout << "Vertices in mesh with ID " << mesh.GetMeshID() << ": " << std::endl;
	for (auto vertex : mesh.GetVertsAsArrays()) {
		std::cout << "(" << vertex[0] << ", " << vertex[1] << ", " << vertex[2] << ")" << std::endl;
	}
}

TEST(_meshInfo, ConvertToOBJCoordinates) {
	// be sure to #include "meshinfo.h", and #include <vector>

	// Prepare the vertices
	std::array<float, 3> vertex_0 = { 34.1, 63.9, 16.5 };
	std::array<float, 3> vertex_1 = { 23.5, 85.7, 45.2 };
	std::array<float, 3> vertex_2 = { 12.0, 24.6, 99.4 };

	// Create an array of vertices
	std::vector<std::array<float, 3>> vertices{ vertex_0, vertex_1, vertex_2 };

	// Create the MeshInfo
	HF::Geometry::MeshInfo mesh(vertices, 3451, "My Mesh");

	// Convert the coordinates
	mesh.ConvertToOBJCoordinates();

	// Note: Program will abort if any coordinate is NaN

	// Display the vertices for mesh
	std::cout << "Vertices in mesh with ID " << mesh.GetMeshID() << ": " << std::endl;
	for (auto vertex : mesh.GetVertsAsArrays()) {
		std::cout << "(" << vertex[0] << ", " << vertex[1] << ", " << vertex[2] << ")" << std::endl;
	}
}

TEST(_meshInfo, PerformRotation) {
	// Prepare the vertices
	std::array<float, 3> vertex_0 = { 34.1, 63.9, 16.5 };
	std::array<float, 3> vertex_1 = { 23.5, 85.7, 45.2 };
	std::array<float, 3> vertex_2 = { 12.0, 24.6, 99.4 };

	// Create an array of vertices
	std::vector<std::array<float, 3>> vertices{ vertex_0, vertex_1, vertex_2 };

	// Create the MeshInfo
	HF::Geometry::MeshInfo mesh(vertices, 3451, "My Mesh");

	// Perform rotation
	float rot_x = 10.0;
	float rot_y = 20.0;
	float rot_z = 30.0;

	mesh.PerformRotation(rot_x, rot_y, rot_z);

	// Display the vertices for mesh
	std::cout << "Vertices in mesh with ID " << mesh.GetMeshID() << ": " << std::endl;
	for (auto vertex : mesh.GetVertsAsArrays()) {
		std::cout << "(" << vertex[0] << ", " << vertex[1] << ", " << vertex[2] << ")" << std::endl;
	}
}

TEST(_meshInfo, GetMeshID) {
	// be sure to #include "meshinfo.h", and #include <vector>

	// Prepare the vertices
	std::array<float, 3> vertex_0 = { 34.1, 63.9, 16.5 };
	std::array<float, 3> vertex_1 = { 23.5, 85.7, 45.2 };
	std::array<float, 3> vertex_2 = { 12.0, 24.6, 99.4 };

	// Create an array of vertices
	std::vector<std::array<float, 3>> vertices{ vertex_0, vertex_1, vertex_2 };

	// Create the MeshInfo
	HF::Geometry::MeshInfo mesh(vertices, 3451, "My Mesh");

	// Use GetMeshID to do an ID match
	int mesh_id = -1;
	if ((mesh_id = mesh.GetMeshID()) == 3451) {
		std::cout << "Retrieved 'My Mesh'" << std::endl;
	}
}

TEST(_meshInfo, GetRawVertices) {
	// be sure to #include "meshinfo.h", and #include <vector>

	// Prepare the vertices. Every three floats represents one vertex.
	std::vector<float> vertices = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
	                            //    0				  1				    2

	// If indices.size() == 3, this means that vertices.size() == 9,
	// and each member of indices represents a std::array<float, 3>
	std::vector<int> indices = { 0, 1, 2 };

	// Create the mesh.
	HF::Geometry::MeshInfo mesh(vertices, indices, 5901, "This Mesh");

	// Retrieve copies of mesh's vertices.
	std::vector<float> vertices_copy_0 = mesh.getRawVertices();
	std::vector<float> vertices_copy_1 = mesh.getRawVertices();

	// Uses std::vector<float>'s operator== to determine member equality
	if (vertices_copy_0 == vertices_copy_1) {
		std::cout << "vertices_copy_0 and vertices_copy_1 share the same elements/permutation." << std::endl;
	}

	// This will demonstrate that vertices_copy_0 and vertices_copy_1 are different container instances
	// Output:	vertices_copy_0 and vertices_copy_1 share the same elements/permutation.
	//			vertices_copy_0 and vertices_copy_1 are different container instances.
	if (&vertices_copy_0 != &vertices_copy_1) {
		std::cout << "vertices_copy_0 and vertices_copy_1 are different container instances." << std::endl;
	}

	// Output all coordinates
	const int total = mesh.NumTris() * 3;
	int offset = 0;

	for (int i = 0; i < total; i++) {
		auto y_offset = static_cast<int64_t>(offset) + 1;
		auto z_offset = static_cast<int64_t>(offset) + 2;

		std::cout << "(" << vertices_copy_0[offset] << ", " 
			<< vertices_copy_0[y_offset] << ", " 
			<< vertices_copy_0[z_offset] << ")" << std::endl;
			offset += 3;
	}
}

TEST(_meshInfo, GetRawIndices) {
	// be sure to #include "meshinfo.h", and #include <vector>

	// Prepare the vertices. Every three floats represents one vertex.
	std::vector<float> vertices = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
	//    0				  1				    2

	// If indices.size() == 3, this means that vertices.size() == 9,
	// and each member of indices represents a std::array<float, 3>
	std::vector<int> indices = { 0, 1, 2 };

	// Create the mesh.
	HF::Geometry::MeshInfo mesh(vertices, indices, 5901, "This Mesh");

	// Retrieve a copy of mesh's index vector
	std::vector<int> indices_copy_0 = mesh.getRawIndices();
	std::vector<int> indices_copy_1 = mesh.getRawIndices();

	// Uses std::vector<int>'s operator== to determine member equality
	if (indices_copy_0 == indices_copy_1) {
		std::cout << "indices_copy_0 and indices_copy_1 share the same elements/permutation." << std::endl;
	}

	// This will demonstrate that indices_copy_0 and indices_copy_1 are different container instances
	// Output:	indices_copy_0 and indices_copy_1 share the same elements/permutation.
	//			indices_copy_0 and indices_copy_1 are different container instances.
	if (&indices_copy_0 != &indices_copy_1) {
		std::cout << "indices_copy_0 and indices_copy_1 are different container instances." << std::endl;
	}

	// Output the indices
	for (auto i : indices_copy_0) {
		std::cout << i << std::endl;
	}
}

TEST(_meshInfo, GetVertsAsArrays) {
	// be sure to #include "meshinfo.h", and #include <vector>

	// Prepare the vertices. Every three floats represents one vertex.
	std::vector<float> vertices = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
	//    0				  1				    2

	// If indices.size() == 3, this means that vertices.size() == 9,
	// and each member of indices represents a std::array<float, 3>
	std::vector<int> indices = { 0, 1, 2 };

	// Create the mesh.
	HF::Geometry::MeshInfo mesh(vertices, indices, 5901, "This Mesh");

	// Retrieve vertices as a vector of coordinates (x, y, z)
	// Useful if your vertices were prepared from a one-dimensional container c, of float
	// (such that c.size() % 3 == 0)
	std::vector<std::array<float, 3>> vert_container = mesh.GetVertsAsArrays();

	// Display the vertices for mesh
	std::cout << "Vertices in mesh with ID " << mesh.GetMeshID() << ": " << std::endl;
	for (auto vertex : mesh.GetVertsAsArrays()) {
		std::cout << "(" << vertex[0] << ", " << vertex[1] << ", " << vertex[2] << ")" << std::endl;
	}
}

TEST(_meshInfo, SetMeshID) {
	// be sure to #include "meshinfo.h", and #include <vector>

	// Prepare the vertices. Every three floats represents one vertex.
	std::vector<float> vertices = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
	//    0				  1				    2

	// If indices.size() == 3, this means that vertices.size() == 9,
	// and each member of indices represents a std::array<float, 3>
	std::vector<int> indices = { 0, 1, 2 };

	// Create the mesh.
	HF::Geometry::MeshInfo mesh(vertices, indices, 5901, "This Mesh");

	// Prepare a new mesh ID.
	int new_mesh_id = 9999;

	// Assign new_mesh_id to mesh.
	mesh.SetMeshID(new_mesh_id);

	// Test for value equality. Will return true.
	if (new_mesh_id == mesh.GetMeshID()) {
		std::cout << "ID assignment successful." << std::endl;
	}
}

TEST(_meshInfo, OperatorEquality) {
	// be sure to #include "meshinfo.h", and #include <vector>

	// Prepare the vertices. Every three floats represents one vertex.
	std::vector<float> vertices_0 = { 11.0, 22.0, 33.0, 44.0, 55.0, 66.0, 77.0, 88.0, 99.0 };
	//	  0                 1                 2

	std::vector<float> vertices_1 = { 11.0, 22.0, 33.0, 44.0, 55.0, 66.0, 77.0, 88.0, 99.0 };
	//	  0                 1                 2

	// indices[0] refers to vertices[0] (beginning of vertex 0)
	// indices[1] refers to vertices[3] (beginning of vertex 1)
	// indices[2] refers to vertices[6] (beginning of vertex 2)
	std::vector<int> indices_0 = { 0, 1, 2 };
	std::vector<int> indices_1 = { 0, 1, 2 };

	HF::Geometry::MeshInfo mesh_0(vertices_0, indices_0, 5901, "This Mesh");
	HF::Geometry::MeshInfo mesh_1(vertices_1, indices_0, 4790, "That Mesh");

	bool equivalent = mesh_0 == mesh_1;		// returns true

	// operator== will determine if two MeshInfo are equal if
	// the Euclidean distance between each matching element in mesh_0 and mesh_1
	// is within 0.001. This means that for all i between vertices_0 and vertices_1 --
	// each x, y, z within
	//		vertices_0[i] and vertices_1[i]
	//	must be within 0.001 to be considered equivalent.

	// Of course, this also means that if
	//		mesh_0.NumVerts() != mesh_1.NumVerts(),
	//	mesh_0 and mesh_1 are not equivalent at all.

	ASSERT_TRUE(equivalent);
}

TEST(_meshInfo, OperatorIndex) {
	// be sure to #include "meshinfo.h", and #include <vector>

	// Prepare the vertices. Every three floats represents one vertex.
	std::vector<float> vertices = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
	//    0				  1				    2

	// If indices.size() == 3, this means that vertices.size() == 9,
	// and each member of indices represents a std::array<float, 3>
	std::vector<int> indices = { 0, 1, 2 };

	// Create the mesh
	HF::Geometry::MeshInfo mesh(vertices, indices, 5901, "This Mesh");

	// Retrieve the desired vertex
	int index = 1;

	// vertex consists of { 23.5, 85.7, 45.2 }
	// which is the second triplet of coordinates within vertices_0.
	auto vertex = mesh[index];

	std::cout << "Retrieved at index " << index << ": (" << vertex[0] << ", "
		<< vertex[1] << ", "
		<< vertex[2] << ")" << std::endl;
}

namespace CInterfaceTests {
	TEST(_OBJLoaderCInterface, LoadOBJ) {
		// Requires #include "objloader_C.h", #include "meshinfo.h"

		// Prepare parameters for LoadOBJ

		// relative path begins where EXE file is located if file_path is not a path to a
		// valid OBJ file, HF::Exceptions::FileNotFound is thrown
		std::string file_path = "big_teapot.obj";

		const int obj_length = file_path.size();

		const float x_rot = 30;
		const float y_rot = 20;
		const float z_rot = 55;

		std::vector<HF::Geometry::MeshInfo>* info = nullptr;

		// Call LoadOBJ
		if (LoadOBJ(file_path.c_str(), obj_length, x_rot, y_rot, z_rot, &info)) {
			std::cout << "LoadOBJ successful" << std::endl;
		}
		else {
			std::cout << "LoadOBJ unsuccessful" << std::endl;
		}

		// Release memory for info once finished with it
		DestroyMeshInfo(info);
	}

	TEST(_OBJLoaderCInterface, StoreMesh) {
		// Requires #include "objloader_C.h", #include "meshinfo.h"
		
		// Prepare parameters for StoreMesh
		std::vector<HF::Geometry::MeshInfo>* info = nullptr;

		int mesh_indices[] = { 0, 1, 2 };
		const int mesh_num_indices = 3;
		float mesh_vertices[] = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
		const int mesh_num_vertices = 9;

		std::string mesh_name = "This mesh";
		const int mesh_id = 0;

		// Call StoreMesh
		if (StoreMesh(&info, mesh_indices, mesh_num_indices, mesh_vertices, mesh_num_vertices, mesh_name.c_str(), mesh_id)) {
			std::cout << "StoreMesh successful" << std::endl;
		}
		else {
			std::cout << "StoreMesh unsuccessful" << std::endl;
		}

		// Release memory for info once finished with it
		DestroyMeshInfo(info);
	}

	TEST(_OBJLoaderCInterface, RotateMesh) {
		// Requires #include "objloader_C.h", #include "meshinfo.h"

		// Prepare parameters for StoreMesh
		std::vector<HF::Geometry::MeshInfo>* info = nullptr;

		int mesh_indices[] = { 0, 1, 2 };
		const int mesh_num_indices = 3;
		float mesh_vertices[] = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
		const int mesh_num_vertices = 9;

		std::string mesh_name = "This mesh";
		const int mesh_id = 0;

		// Call StoreMesh
		if (StoreMesh(&info, mesh_indices, mesh_num_indices, mesh_vertices, mesh_num_vertices, mesh_name.c_str(), mesh_id)) {
			std::cout << "StoreMesh successful" << std::endl;
		}
		else {
			std::cout << "StoreMesh unsuccessful" << std::endl;
		}

		// Prepare desired rotation values
		const float x_rot = 10;
		const float y_rot = 10;
		const float z_rot = 20;

		// Call RotateMesh
		if (RotateMesh(info, x_rot, y_rot, z_rot)) {
			std::cout << "RotateMesh successful" << std::endl;
		}
		else {
			std::cout << "RotateMesh unsuccessful" << std::endl;
		}

		// Release memory for info once finished with it
		DestroyMeshInfo(info);
	}
	
	TEST(_OBJLoaderCInterface, DestroyMeshInfo) {
		// Requires #include "objloader_C.h", #include "meshinfo.h"

		// Prepare parameters for LoadOBJ

		// relative path begins where EXE file is located if file_path is not a path to a
		// valid OBJ file, HF::Exceptions::FileNotFound is thrown
		std::string file_path = "big_teapot.obj";

		const int obj_length = file_path.size();

		const float x_rot = 30;
		const float y_rot = 20;
		const float z_rot = 55;

		std::vector<HF::Geometry::MeshInfo>* info = nullptr;

		// Call LoadOBJ
		if (LoadOBJ(file_path.c_str(), obj_length, x_rot, y_rot, z_rot, &info)) {
			std::cout << "LoadOBJ successful" << std::endl;
		}
		else {
			std::cout << "LoadOBJ unsuccessful" << std::endl;
		}

		// Release memory for info once finished with it
		DestroyMeshInfo(info);
	}
}