#include <gtest/gtest.h>
#include <objloader.h>
#include <meshinfo.h>
#include <HFExceptions.h>
#include <string>
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
