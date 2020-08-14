#include "gtest/gtest.h"

#include "performance_testing.h"
#include <embree_raytracer.h>
#include <objloader.h>
#include <meshinfo.h>
#include <view_analysis.h>

#include <vector>
#include <array>
#include <ostream>
#include <fstream>


using HF::RayTracer::HitStruct;
using HF::RayTracer::EmbreeRayTracer;
using HF::Geometry::MeshInfo;
using std::vector;
using std::array;

inline int count_hits(vector<HitStruct>& results) {
	int hits = 0;
	for (const auto& result : results)
		if (result.DidHit())
			++hits;

	return hits;
}

void PrintDirections(const std::vector < std::array<float, 3>> directions) {
	for (const auto& direction : directions)
		printf("(%f,%f,%f)", directions[0], directions[1], directions[2]);

}

struct ModelAndStart {
	std::array<float, 3> start;
	EmbreeRayTracer PreciseERT;
	EmbreeRayTracer StandardERT;

	int verts = 0;
	int triangles = 0;
	string model_name;

	ModelAndStart(std::array<float, 3> start_point, string model, bool flip_z = false) {

		std::cout << "Loading " << model << std::endl;

		std::vector<MeshInfo> MI = HF::Geometry::LoadMeshObjects(model, HF::Geometry::GROUP_METHOD::ONLY_FILE, flip_z);
		for (auto& m : MI)
		{
			verts += m.NumVerts();
			triangles += m.NumTris();
		}

		StandardERT = EmbreeRayTracer(MI, false);
		PreciseERT = EmbreeRayTracer(MI, true);
		start = start_point;
		model_name = model;
	};
};

inline void WriteToCSV(std::ofstream& file, const std::vector<string>& strings_to_write) {
	const int n = strings_to_write.size();
	for (int i = 0; i < strings_to_write.size(); i++) {
		file << strings_to_write[i] << ((i != n - 1) ? "," : "\n");
	}
}

// This will run once for every model and every raycount
TEST(Performance, CustomTriangleIntersection) {
	const std::string filename = "CustomTriangleIntersectionResults.csv";

	// Number of trials is based on number of elements here
	const vector<int> raycount = {
		10000,	10000,	10000,
		50000,	50000, 	50000,
		100000,	100000, 100000,
		500000,	500000,	500000,
		1000000,1000000,1000000,
		5000000,5000000,5000000,
		10000000,10000000, 10000000
	};

	printf("Loading Models...\n");
	vector<ModelAndStart> models = {
		ModelAndStart({0,0,1},  "plane.obj", true),
		ModelAndStart({-4.711,1.651,-14.300},  "sibenik.obj", true),
	//	ModelAndStart({-4.711,1.651,-14.300},  "sibenik_subdivided.obj", true),
		ModelAndStart({0.007,-0.001,0.093},  "sponza.obj", true),
		ModelAndStart({0,0,1},  "energy_blob_zup.obj"),
		ModelAndStart({833.093,546.809,288.125},  "Weston_Analysis.obj"),
		//	ModelAndStart({2532.320,-19.040,45.696},  "ButchersDenFinal.obj", true),
			//ModelAndStart({0,0,1},  "zs_abandonded_mall.obj", true),
		//	ModelAndStart({0,0,1},  "zs_amsterdam.obj", true),
		//	ModelAndStart({0,0,1},  "zs_comfy.obj", true),
		//	ModelAndStart({0,0,1},  "dragon.obj", true),
		//	ModelAndStart({44.218,-39.946,15.691},  "mountain.obj", true)
	};

	const int num_trials = raycount.size();

	// Create Watches
	std::vector<StopWatch> watches(num_trials);

	printf("GeneratingDirections...\n");
	vector < vector<array<float, 3>>> directions;
	vector < vector<array<float, 3>>> origins;
	for (int rc : raycount) {
		directions.push_back(HF::ViewAnalysis::FibbonacciDistributePoints(rc, 90, 90));
	}
	//PrintDirections(directions[0]);

	vector<std::string> RowHeaders = {
		"Trial Number",
		"Model",
		"Rays",
		"Standard Hits",
		"Precise Hits",
		"Time Standard (ms)",
		"Time Precise (ms)",
		"Vertices",
		"Triangles"
	};
	// Open CSV
	std::ofstream csv_output;
	csv_output.open("filename");
	WriteToCSV(csv_output, RowHeaders);
	int k = 0;
	for (auto& mas : models) {
		// Get things that wil stay constant
		auto tris = mas.triangles;
		auto verts = mas.verts;
		const auto& origin = mas.start;

		std::vector<std::string> output = {
			"",
			mas.model_name,
			"",
			"",
			"",
			"",
			"",
			std::to_string(mas.verts),
			std::to_string(mas.triangles)
		};
		printf("Conducting Tests for ");
		std::cout << mas.model_name << std::endl;
		for (int i = 0; i < num_trials; i++) {
			StopWatch standard_watch;
			StopWatch precise_watch;

			const auto& dirs = directions[i];
			const auto rc = dirs.size();
			// Create arrays of origins and directions
			const vector<array<float, 3>> origins(rc, origin);

			printf("Firing %i Rays... \n", rc);

			// Conduct Precise Check
			precise_watch.StartClock();
			vector<HitStruct> precise_results = mas.PreciseERT.FireAnyRayParallel(origins, dirs, -1.0f, true, false);
			precise_watch.StopClock();

			// Conduct standard check
			standard_watch.StartClock();
			vector<HitStruct> results = mas.StandardERT.FireAnyRayParallel(origins, dirs, -1.0f, false, false);
			standard_watch.StopClock();


			// Update output
			output[0] = std::to_string(k++);
			output[2] = std::to_string(rc);
			output[3] = std::to_string(count_hits(results));
			output[4] = std::to_string(count_hits(precise_results));
			output[5] = std::to_string(static_cast<double>(standard_watch.GetDuration()) / 1000000.0);
			output[6] = std::to_string(static_cast<double>(precise_watch.GetDuration()) / 1000000.0);

			// Write row of CSV
			WriteToCSV(csv_output, output);
		}
	}
	csv_output.close();
}