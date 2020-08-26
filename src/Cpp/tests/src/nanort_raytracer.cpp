#include <gtest/gtest.h>

#include <string>
#include <objloader.h>
#include <meshinfo.h>
#include <embree_raytracer.h>
#include <robin_hood.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <view_analysis.h>
#include <HFExceptions.h>

#include <objloader_C.h>
#include <raytracer_C.h>


#include "RayRequest.h"

#include "performance_testing.h"

// [nanoRT]
#define NANORT_USE_CPP11_FEATURE  // MUST be defined for nanort.h 
#include "nanort.h"
#include "ray_data.h"
using namespace HF::nanoGeom;
// end [nanoRT]

using namespace HF::Geometry;
using namespace HF::RayTracer;
using std::vector;
using std::array;
using std::string;
using std::cerr;
using std::endl;

using HF::Geometry::MeshInfo;
using namespace HF::Geometry;


TEST(_nanoRayTracer, Edge_Vert_Intersection) {
	// Check to see if nanort will find the box

	std::string objFilename = "VisibilityTestCases.obj";

	// Basic setup of nanoRT interface
	bool ret = false;
	Mesh mesh;
	ret = LoadObj(mesh, objFilename.c_str());
	nanort::BVHAccel<double> accel;
	accel = nanoRT_BVH(mesh);

	nanoRT_Data nanoRTdata(mesh);

	// Set the comparison to embree
	std::vector<std::array<double, 3>> origins = { {19, 10, 15},
												  {20, 10, 15} };

	// Define direction of ray
	nanoRTdata.ray.dir[2] = -1.0; // Change z
	double height = NAN;

	for (auto& origin : origins) {
		nanoRTdata.ray.org[0] = origin[0];
		nanoRTdata.ray.org[1] = origin[1];
		nanoRTdata.ray.org[2] = origin[2];
		bool hit = nanoRT_Intersect(mesh, accel, nanoRTdata);
		height = nanoRTdata.point[2];
		ASSERT_EQ(height, 10);
	}

}

TEST(_nanoRayTracer, nanoRayTolerance) {

	std::string objFilename = "energy_blob_zup.obj";

	// Basic setup of nanoRT interface
	bool ret = false;
	Mesh mesh;
	ret = LoadObj(mesh, objFilename.c_str());
	nanort::BVHAccel<double> accel;
	accel = nanoRT_BVH(mesh);

	nanoRT_Data nanoRTdata(mesh);

	// Set the comparison to embree
	std::vector<std::array<double, 3>> origins = { {-30.01, 0.0, 50.0},
												  {-30.01, 0.0, 150.1521},
												  {-30.01, 0.0, 85.01311} };

	// Setup the ray
	// Set origin of ray which defaults to all 0's
	nanoRTdata.ray.org[0] = -30.01; // Change x
	nanoRTdata.ray.org[2] = 0.0; // Change z

	// Define direction of ray
	nanoRTdata.ray.dir[2] = -1.0; // Change z
	double height = NAN;

	for (auto& origin : origins) {

		nanoRTdata.ray.org[2] = origin[2];
		bool hit = nanoRT_Intersect(mesh, accel, nanoRTdata);
		height = nanoRTdata.point[2];
		// embree: 1.06882095          1.06833649 
		// nanoRT: 1.0683273067522734  1.0683273067522521
	}

	nanoRTdata.ray.org[0] = -30.0; // Change x
	nanoRTdata.ray.org[2] = 20.0; // Change z

	// We pass it our custom class that contains a built-in hit point member that will be modified in place
	bool hit = nanoRT_Intersect(mesh, accel, nanoRTdata);

	double diff = nanoRTdata.hit.t - 18.931174758804396;
	ASSERT_LE(diff, 0.00000001);

}

TEST(_nanoRayTracer, nanoRayPerformance) {

	//std::string objFilename = "energy_blob_zup.obj"; // 3k ray/ms
	std::string objFilename = "Weston_Analysis_z-up.obj"; // 580 ray/ms
	//std::string objFilename = "Weston_3copies.obj"; // 153 ray/ms // set z to 600

	// Basic setup of nanoRT interface
	bool ret = false;
	Mesh mesh;
	ret = LoadObj(mesh, objFilename.c_str());
	nanort::BVHAccel<double> accel;
	accel = nanoRT_BVH(mesh);

	nanoRT_Data nanoRTdata(mesh);
	nanoRTdata.ray.org[2] = 600;
	nanoRTdata.ray.dir[2] = -1;

	// Number of trials is based on number of elements here
	vector<int> raycount = { 0 };
	const int num_trials = raycount.size();

	// Create Watches
	std::vector<StopWatch> watches(num_trials);

	auto& watch = watches[0];
	watch.StartClock();
	double dist_sum = 0; // Sum of hits to make sure loop is not optimized away
	// Do it in a loop for checking performance
	for (float i = -300; i < 300; i++) {
		for (float j = -300; j < 300; j++) {
			nanoRTdata.ray.org[0] = i * 0.01;
			nanoRTdata.ray.org[1] = j * 0.01;
			// We pass it our custom class that contains a built-in hit point member that will be modified in place
			bool hit = nanoRT_Intersect(mesh, accel, nanoRTdata);
			dist_sum += nanoRTdata.point[2];
			raycount[0]++;
		}
	}
	watch.StopClock();
	PrintTrials(watches, raycount, "rays with nanoRT");
	std::cout << " Total distance of rays: " << dist_sum << std::endl;
}

