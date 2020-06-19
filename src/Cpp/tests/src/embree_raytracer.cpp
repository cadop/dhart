#include <gtest/gtest.h>
#include <meshinfo.h>
#include <string>
#include <objloader.h>
#include <embree_raytracer.h>
#include <robin_hood.h>
#include <cmath>

using namespace HF::Geometry;
using namespace HF::RayTracer;
using std::vector;
using std::array;
using std::cerr;
using std::endl;

/// <summary>
/// Create a new raytracer from a basic 10x10 plane centered on the origin. 
/// </summary>
inline EmbreeRayTracer CreateRTWithPlane(){

	const vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};

	const vector<int> plane_indices{3, 1, 0, 2, 3, 0};
	return EmbreeRayTracer(vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});
}

/// <summary>
/// Calc distance between two arrays.
/// </summary>
inline float Distance(const array<float,3> & p1, const array<float,3> & p2) {
	return sqrt(
		pow(p1[0] - p2[0], 2)
		+ pow(p1[1] - p2[1], 2)
		+ pow(p1[2] - p2[2], 2)
	);
}

TEST(_EmbreeRayTracer, HashAlgorithm) {
	robin_hood::unordered_map<std::array<float, 3>, int> floatmap;

	std::array<float, 3> key1 = { 1,2,3 };
	std::array<float, 3> key2 = { 1.01,2.01,3.01 };

	floatmap[key1] = 1;
	floatmap[key2] = 2;

	EXPECT_EQ(floatmap[key1], 1);
	EXPECT_EQ(floatmap[key2], 2);
}

TEST(_EmbreeRayTracer, Init) {
	std::string teapot_path = "teapot.obj";
	auto geom = HF::Geometry::LoadMeshObjects(teapot_path, HF::Geometry::ONLY_FILE);

	auto k = HF::RayTracer::EmbreeRayTracer(geom);
}

TEST(_EmbreeRayTracer, Copy) {
	std::string teapot_path = "teapot.obj";
	auto geom = HF::Geometry::LoadMeshObjects(teapot_path, HF::Geometry::ONLY_FILE);
	auto k = HF::RayTracer::EmbreeRayTracer(geom);

	// If the copy didn't work, then this operation would throw
	auto rt2 = k;
	rt2.FireOcclusionRay(std::array<float, 3>{1, 1, 1}, std::array<float, 3>{1, 1, 1});
}

TEST(_EmbreeRayTracer, OcclusionRays) {
	std::string teapot_path = "big_teapot.obj";
	auto geom = HF::Geometry::LoadMeshObjects(teapot_path, HF::Geometry::ONLY_FILE);
	auto k = HF::RayTracer::EmbreeRayTracer(geom);

	// All of these rays should hit since the origin is inside of the teapot
	std::array<float, 3> origin{ 0,0,1 };
	std::vector<std::array<float, 3>> directions = {
		{0,0,1},
		{0,1,0},
		{1,0,0},
		{-1,0,0},
		{0,-1,0},
		{0,0,-1},
	};
	for (auto& dir : directions)
		EXPECT_TRUE(k.FireOcclusionRay(origin, dir));
}

TEST(_EmbreeRayTracer, StandardRays) {
	std::string teapot_path = "big_teapot.obj";
	auto geom = HF::Geometry::LoadMeshObjects(teapot_path, HF::Geometry::ONLY_FILE);
	auto k = HF::RayTracer::EmbreeRayTracer(geom);

	// All of these rays should hit since the origin is inside of the teapot
	const std::vector<std::array<float, 3>> directions = {
		{0,0,1},
		{0,1,0},
		{1,0,0},
		{-1,0,0},
		{0,-1,0},
		{0,0,-1},
	};
	for (auto& dir : directions) {
		std::array<float, 3> origin{ 0,0,1 };
		std::cerr << "(" << dir[0] << "," << dir[1] << "," << dir[2] << ")" << std::endl;
		EXPECT_TRUE(k.FireRay(origin, dir));
	}
}

TEST(_EmbreeRayTracer, HitPointsAreAccurate) {
	std::string plane_path = "plane.obj";
	auto geom = HF::Geometry::LoadMeshObjects(plane_path, HF::Geometry::ONLY_FILE);
	auto k = HF::RayTracer::EmbreeRayTracer(geom);

	// All of these rays should hit since the origin is inside of the teapot
	std::vector<std::array<float, 3>> origins = {
		{0,0,1},
		{0,1,1},
		{1,0,1},
		{-1,0,1},
		{0,-1,1},
		{0,0,1},
	};

	const std::array<float, 3> direction{ 0,0,-1 };
	float height = NAN;
	for (auto& origin : origins) {
		std::cerr << "(" << origin[0] << "," << origin[1] << "," << origin[2] << ")" << std::endl;
		EXPECT_TRUE(k.FireRay(origin, direction));

		if (isnan(height))
			height = origin[2];
		else
			EXPECT_NEAR(height, origin[2], 0.001);
	}
}

// TODO: Add a distance check to this?
TEST(_EmbreeRayTracer, FireRays) {

	// Create plane
	const std::vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create RayTracer
	EmbreeRayTracer ert(vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

	// Create an array of directions all containing {0,0,-1}
	std::vector<std::array<float, 3>> directions(10, std::array<float, 3>{0, 0, -1});
	
	// Create an array of origin points moving further to the left with each point
	std::vector<std::array<float, 3>> origins(10);
	for (int i = 0; i < 10; i++) origins[i] = std::array<float, 3>{static_cast<float>(1.99*i), 0, 1};

	// Fire every ray. Results should all be true and be within a certain distance of zero;
	auto results = ert.FireRays(origins, directions);

	// Print results
	std::cerr << "[";
	for (int i = 0; i < 10; i++) {
		if (results[i])
			std::cerr << "(" << origins[i][0] << ", " << origins[i][1] << ", " << origins[i][2] << ")";
		else
			std::cerr << "Miss";

		if (i != 9) std::cerr << ", ";

		// Test that the ray intersected, and it isn't far from where it should have hit.
		if (i < 6) {
			ASSERT_TRUE(results[i]);

			float dist = Distance(origins[i], std::array<float, 3>{static_cast<float>(i) * 1.99f, 0, 0});
			ASSERT_NEAR(dist, 0, 0.0001);
		}
		else ASSERT_FALSE(results[i]);
	}
	std::cerr << "]" << std::endl;

}

TEST(_EmbreeRayTracer, FireOcclusionRays) {

	// Create Plane
	const std::vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create RayTracer
	EmbreeRayTracer ert(vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

	// Create an array of directions all containing {0,0,-1}
	std::vector<std::array<float, 3>> directions(10, std::array<float, 3>{0, 0, -1});

	// Create an array of origins with the first 5 values being above the plane and the last
	// five values being under it.
	std::vector<std::array<float, 3>> origins(10);
	for (int i = 0; i < 5; i++) origins[i] = std::array<float, 3>{0.0f, 0.0f, 1.0f};
	for (int i = 5; i < 10; i++) origins[i] = std::array<float, 3>{0.0f, 0.0f, -1.0f};

	// Fire every ray.
	std::vector<bool> results = ert.FireOcclusionRays(origins, directions);

	// Iterate through all results to print them
	std::cerr << "[";
	for (int i = 0; i < 10; i++) {

		// Print true if the ray intersected, false otherwise
		if (results[i]) std::cout << "True";
		else std::cerr << "False";
		
		// Add a comma if it's not the last member
		if (i != 9) std::cerr << ", ";

		if (i < 5) ASSERT_TRUE(results[i]);
		else ASSERT_FALSE(results[i]);
	}
	std::cerr << "]" << std::endl;
}

TEST(_EmbreeRayTracer, FireRay) {
	// Create Plane
	const vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create RayTracer
	EmbreeRayTracer ert(vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

	float x = 0; float y = 0;float z = 1;
	bool res;

	// Fire a ray straight down and ensure it connects with a distance of 1 (within a certain tolerance)
	res = ert.FireRay(x, y, z, 0, 0, -1);
	if (res) std::cerr << "(" << x << ", " << y << ", " << z << ")" << std::endl;
	else std::cerr << "Miss" << std::endl;

	ASSERT_TRUE(res);
	ASSERT_NEAR(Distance(std::array<float, 3>{x, y, z}, std::array<float, 3>{0, 0, 0}), 0, 0.0001);

	x = 0; y = 0; z = 1;
	// Fire a ray straight up and ensure it misses
	res = ert.FireRay(x, y, z, 0, 0, 1);
	if (res) std::cerr << "(" << x << ", " << y << ", " << z << ")" << std::endl;
	else std::cerr << "Miss" << std::endl;

	ASSERT_FALSE(res);
}

TEST(_EmbreeRayTracer, FireRayArrayOverload) {
	// Create Plane
	const vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create RayTracer
	EmbreeRayTracer ert(vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

	// Fire a ray straight down and ensure it connects with a distance of 1 (within a certain tolerance)
	std::array<float, 3> origin{ 0,0,1 };
	bool res = ert.FireRay(
		origin,
		std::array<float, 3>{0, 0, -1}
	);

	// Print Results
	if (res) std::cerr << "(" << origin[0] << ", " << origin[1] << ", " << origin[2] << ")" << std::endl;
	else std::cerr << "Miss" << std::endl;

	ASSERT_TRUE(res);
	ASSERT_NEAR(Distance(origin, std::array<float, 3>{0, 0, 0}), 0, 0.0001);

	// Fire a ray straight up and ensure it misses
	origin = std::array<float,3>{ 0,0,1 };
	res = ert.FireRay(
		origin,
		std::array<float, 3>{0, 0, 1}
	);

	// Print Results
	if (res) std::cerr << "(" << origin[0] << ", " << origin[1] << ", " << origin[2] << ")" << std::endl;
	else std::cerr << "Miss" << std::endl;

	ASSERT_FALSE(res);
}

TEST(_EmbreeRayTracer, Intersect){
	// Create Plane
	const vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create RayTracer
	EmbreeRayTracer ert(vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

	HitStruct res;

	// Fire a ray straight down
	res = ert.Intersect(0, 0, 1, 0, 0, -1);
	
	// Print distance if it connected
	if (res.DidHit()) std::cerr << res.distance << std::endl;
	else std::cerr << "Miss" << std::endl;

	ASSERT_TRUE(res.DidHit());
	ASSERT_NEAR(res.distance, 1, 0.0001);

	// Fire a ray straight up and ensure it misses
	res = ert.Intersect(0, 0, 1, 0, 0, 1);
	if (res.DidHit()) std::cerr << res.distance << std::endl;
	else std::cerr << "Miss" << std::endl;

	ASSERT_FALSE(res.DidHit());
}

TEST(_EmbreeRayTracer, FireAnyRay) {
	// Create Plane
	const std::vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create RayTracer
	EmbreeRayTracer ert(vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});
	
	// Create a vector of direction and origin arrays. 
	std::array<float, 3> origin{ 0,0,1 };
	std::array<float, 3> direction{ 0,0,-1 };

	bool res = false; float out_dist = -1; int out_id = -1;
	
	// Fire a ray straight down 	
	res = ert.FireAnyRay(origin, direction, out_dist, out_id);
	ASSERT_TRUE(res);
	ASSERT_NEAR(out_dist, 1, 0.0001);
	
	// Print its distance if it connected
	if (res) std::cerr << out_dist << std::endl;
	else std::cerr << "Miss" << std::endl;

	// Fire a ray straight up and ensure it misses
	res = ert.FireAnyRay(origin, origin, out_dist, out_id);
	ASSERT_FALSE(res);
	
	// Print its distance if it connected
	if (res) std::cerr << out_dist << std::endl;
	else std::cerr << "Miss" << std::endl;
}

TEST(_EmbreeRayTracer, FireAnyOcclusionRay){
	// Create Plane
	const vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create RayTracer
	EmbreeRayTracer ert(vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

	// Fire a ray straight down
	bool res = ert.FireAnyOcclusionRay(
		std::array<float, 3>{0, 0, 1},
		std::array<float, 3>{0, 0, -1}
	);

	ASSERT_TRUE(res);
	if (res) std::cerr << "True" << std::endl;
	else std::cerr << "False" << std::endl;

	// Fire a ray straight up
	res = ert.FireAnyOcclusionRay(
		std::array<float, 3>{0, 0, 1},
		std::array<float, 3>{0, 0, 1}
	);

	ASSERT_FALSE(res);
	if (res) std::cerr << "True" << std::endl;
	else std::cerr << "False" << std::endl;
}

TEST(_EmbreeRayTracer, FireOcclusionRayArray) {
	// Create Plane
	const vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create RayTracer
	EmbreeRayTracer ert(vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});

	// Fire a ray straight down
	bool res = ert.FireOcclusionRay(
		std::array<float, 3>{0, 0, 1},
		std::array<float, 3>{0, 0, -1}
	);

	ASSERT_TRUE(res);
	if (res) std::cerr << "True" << std::endl;
	else std::cerr << "False" << std::endl;

	// Fire a ray straight up
	res = ert.FireOcclusionRay(
		std::array<float, 3>{0, 0, 1}, 
		std::array<float, 3>{0, 0, 1}
	);
	
	ASSERT_FALSE(res);
	if (res) std::cerr << "True" << std::endl;
	else std::cerr << "False" << std::endl;
}

TEST(_EmbreeRayTracer, FireOcclusionRay) {
	// Create Plane
	const vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create RayTracer
	EmbreeRayTracer ert(vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});
		
	// Fire a ray straight down
	bool res = ert.FireOcclusionRay(0,0,1,0,0,-1);
	ASSERT_TRUE(res);
	if (res) std::cerr << "True" << std::endl;
	else std::cerr << "False" << std::endl;

	// Fire a ray straight up
	res = ert.FireOcclusionRay(0,0,1,0,0,1);
	ASSERT_FALSE(res);
	if (res) std::cerr << "True" << std::endl;
	else std::cerr << "False" << std::endl;
}
