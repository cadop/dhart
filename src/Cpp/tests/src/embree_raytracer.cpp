#include <gtest/gtest.h>
#include <meshinfo.h>
#include <string>
#include <objloader.h>
#include <embree_raytracer.h>
#include <robin_hood.h>
#include <cmath>
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
		std::cout << "(" << dir[0] << "," << dir[1] << "," << dir[2] << ")" << std::endl;
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
		std::cout << "(" << origin[0] << "," << origin[1] << "," << origin[2] << ")" << std::endl;
		EXPECT_TRUE(k.FireRay(origin, direction));

		if (isnan(height))
			height = origin[2];
		else
			EXPECT_NEAR(height, origin[2], 0.001);
	}
}