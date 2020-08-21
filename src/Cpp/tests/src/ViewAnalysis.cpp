#include "gtest/gtest.h"

#include <gtest/gtest.h>
#include <array>
#include <string>
#include <numeric>

#include <graph_generator.h>
#include <embree_raytracer.h>
#include <view_analysis.h>
#include <objloader.h>
#include <meshinfo.h>
#include <graph.h>
#include <edge.h>
#include <node.h>

#include <view_analysis_C.h>
#include <raytracer_C.h>
#include <objloader_C.h>
#include <HFExceptions.h>

using namespace HF;
using HF::SpatialStructures::Graph;
using HF::SpatialStructures::Node;
using HF::RayTracer::EmbreeRayTracer;
using HF::Geometry::MeshInfo;
using namespace HF::Geometry;

// The Utah teapot scaled up https://en.wikipedia.org/wiki/File:Utah_teapot_simple_2.png
// to about 6.7 x 4 x 3.14. Has 3,238 vertices, 6,320 triangles. Copied to the directory
// of the test executable when the project is built.
std::string big_teapot_path = "big_teapot.obj";
/*!
TEST(_ViewAnalysis, AllRaysHit) {
	auto MI = LoadMeshObjects(big_teapot_path);
	EmbreeRayTracer ERT(MI);
	std::vector<Node> points{ {0,0,0}, {10000, 10000, 10000} };

	std::vector<float> scores = ViewAnalysis::SphericalRayshootWithAnyRTForDistance(
		ERT,
		points,
		10000,
		50.0f,
		70.0f,
		1.0f,
		ViewAnalysis::AGGREGATE_TYPE::SUM
	);

	ASSERT_LT(scores[1], scores[0]);
}

TEST(_ViewAnalysis, FibbonacciGeneratesRightAmtOfRays) {
	ASSERT_EQ(ViewAnalysis::FibbonacciDistributePoints(100).size(), 100);
}

TEST(_ViewAnalysis, AggregateDoesntReturnInfinite) {
	auto MI = LoadMeshObjects(big_teapot_path);
	EmbreeRayTracer ERT(MI);

	std::vector<Node> points{ {0,0,0}, {10000, 10000, 10000} };
	auto scores = ViewAnalysis::SphericalRayshootWithAnyRTForDistance(
		ERT,
		points,
		10000,
		1.0f,
		50.0f,
		70.0f,
		ViewAnalysis::AGGREGATE_TYPE::AVERAGE
	);
	for (auto score : scores)
		ASSERT_TRUE(isnormal(score) || score == 0);
}

TEST(_ViewAnalysis, FibbonacciDistributePoints) {
	int size = 8;

	// For brevity
	using HF::ViewAnalysis::FibbonacciDistributePoints;

	// Generate points without any limitations on fov
	auto points = FibbonacciDistributePoints(size, 90.0f, 90.0f);
	ASSERT_TRUE(points.size() == size);

	// Print number of points
	std::cerr << "Number of Points:" << points.size() << std::endl;

	// Iterate through and print every point.
	std::cerr << "[";
	for (int i = 0; i < size; i++) {
		const auto& point = points[i];
		std::cerr << "(" << point[0] << ", " << point[1] << ", " << point[2] << ")";
		if (i != size - 1) std::cerr << ", ";
	}

	std::cerr << "]" << std::endl;
}

TEST(_ViewAnalysis, AggregateAverage)
{
	// Requires #include <numeric>

	// Use this to save some space.
	using HF::ViewAnalysis::AGGREGATE_TYPE;
	using HF::ViewAnalysis::Aggregate;

	// Undef these since they will prevent us from calling numericlimits
#undef min
#undef max

// Define values
	std::vector<int> values = { 1, 2, 3, 4, 5 };

	// Calculate Average
	float total = 0.0f;	int count = 0;
	for (int val : values) {
		count += 1;
		Aggregate(total, val, AGGREGATE_TYPE::AVERAGE, count);
	}
	std::cerr << "Average: " << total << std::endl;
	ASSERT_EQ(total, 3);

	// Calculate Sum
	total = 0.0f;
	for (int val : values) Aggregate(total, val, AGGREGATE_TYPE::SUM);
	std::cerr << "Sum: " << total << std::endl;
	ASSERT_EQ(total, 15);

	// Calculate Max. Start total at lowest possible float value to ensure
	// it overwritten by the first element.
	total = std::numeric_limits<float>::min();
	for (int val : values) Aggregate(total, val, AGGREGATE_TYPE::MAX);
	std::cerr << "Max: " << total << std::endl;
	ASSERT_EQ(total, 5);

	// Calculate Min. Start total at highest possible float value to ensure
	// it overwritten by the first element.
	total = (std::numeric_limits<float>::max());
	for (int val : values) Aggregate(total, val, AGGREGATE_TYPE::MIN);
	std::cerr << "Min: " << total << std::endl;
	ASSERT_EQ(total, 1);

	// Calculate Count
	total = 0.0f;
	for (int val : values) Aggregate(total, val, AGGREGATE_TYPE::COUNT);
	std::cerr << "Count: " << total << std::endl;
	ASSERT_EQ(total, 5);
}

TEST(_ViewAnalysis, SphericalViewAnalysis) {
	struct SampleResults {
		float dist = -1.0f;
		inline void SetHit(
			const std::array<float, 3>& node,
			const std::array<float, 3>& direction,
			float distance,
			int meshID
		) {
			dist = distance;
		}
	};

	// Use this so we can fit within 80 characters
	using HF::ViewAnalysis::SphericalViewAnalysis;
	using HF::RayTracer::EmbreeRayTracer;

	// Create Plane
	const std::vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create RayTracer
	EmbreeRayTracer ert(std::vector<MeshInfo>{
		MeshInfo(plane_vertices, plane_indices, 0, " ")}
	);

	// Define observer points
	std::vector<std::array<float, 3>> points{
		{0,0,0}, {1,1,0}, {1,2,0}, {1000, 1000, 0}
	};

	// Perform View Analysis
	int num_rays = 50;
	auto results = SphericalViewAnalysis<SampleResults>(ert, points, num_rays);

	// Determine how many directions there were
	int num_directions = results.size() / points.size();

	// Construct a vector from the results of the first node
	std::vector<SampleResults> first_results(
		results.begin(), results.begin() + num_directions
	);

	// Print Results
	std::cerr << "(";
	for (int i = 0; i < first_results.size(); i++) {
		const auto& result = first_results[i];
		std::cerr << result.dist;

		if (i != first_results.size() - 1) std::cerr << ", ";
	}
	std::cerr << ")" << std::endl;

	// Compare results with expected results
	std::vector<float> first_five_expected_results{ -1, 7.35812, -1, -1, 3.70356, -1 };
	std::vector<float> last_five_expected_results = { -1, 6.80486, -1, 5.12012, -1 };
	std::vector<SampleResults> actual_first_five(first_results.begin(), first_results.begin() + 5);
	std::vector<SampleResults> actual_last_five(first_results.end() - 5, first_results.end());

	for (int i = 0; i < 5; i++) {
		ASSERT_NEAR(actual_first_five[i].dist, first_five_expected_results[i], 0.00001f);
		ASSERT_NEAR(actual_last_five[i].dist, last_five_expected_results[i], 0.00001f);
	}
}

TEST(_ViewAnalysis, ViewAnalysisAggregate) {
	// Use this so we can fit within 80 characters
	using HF::ViewAnalysis::SphericalRayshootWithAnyRTForDistance;
	using HF::ViewAnalysis::AGGREGATE_TYPE;

	// Create Plane
	const std::vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create RayTracer
	EmbreeRayTracer ert(std::vector<MeshInfo>{
		MeshInfo(plane_vertices, plane_indices, 0, " ")}
	);

	// Define observer points
	std::vector<std::array<float, 3>> points{
		{0,0,0}, {10,10,0}, {20,20,0}, {30, 30, 0}
	};

	// Perform View Analysis and sum the distance to all intersections
	// for every node
	int num_rays = 2000;
	auto results = SphericalRayshootWithAnyRTForDistance(
		ert, points, num_rays, 90.0f, 90.0f, 1.7f, AGGREGATE_TYPE::SUM
	);

	// Print Results
	std::cerr << "(";
	for (int i = 0; i < results.size(); i++) {
		std::cerr << results[i];

		if (i != results.size() - 1)
		{
			// Since each node is getting further away, they should
			// have a lower score than the node before them.
			ASSERT_GT(results[i], results[i + 1]);
			std::cerr << ", ";
		}
	}
	std::cerr << ")" << std::endl;
}

TEST(C_ViewAnalysisCInterface, SphericalViewAnalysisAggregate) {
	// Create Plane
	const std::vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create and allocate a new instacnce of meshinfo
	std::vector<MeshInfo>* MI;
	auto MIR = StoreMesh(
		&MI,
		plane_indices.data(),
		plane_indices.size(),
		plane_vertices.data(),
		plane_vertices.size(),
		"",
		0
	);
	ASSERT_EQ(MIR, HF::Exceptions::HF_STATUS::OK);

	// Create a new raytracer
	EmbreeRayTracer* ert;
	CreateRaytracer(MI, &ert);

	// Create Nodes
	std::vector<Node> nodes = {
		Node(0,0,1),
		Node(0,0,2),
		Node(0,0,3),
	};

	// Make settings
	float max_rays = 10000;
	float up_fov = 90;
	float down_fov = 90;
	float height = 1.7f;
	AGGREGATE_TYPE AT = AGGREGATE_TYPE::AVERAGE;

	std::vector<float>* scores;
	float* scores_ptr;
	int scores_size;

	// Run View Analysis
	auto result = SphereicalViewAnalysisAggregate(
		ert,
		nodes.data(),
		nodes.size(),
		max_rays,
		up_fov,
		down_fov,
		height,
		AT,
		&scores,
		&scores_ptr,
		&scores_size
	);
	ASSERT_EQ(result, HF::Exceptions::HF_STATUS::OK);
	ASSERT_EQ(scores->size(), scores_size);

	// print Results
	for (int i = 0; i < scores->size(); i++)
		std::cerr << (*scores)[i] << std::endl;

	// Deallocate Memory
	DestroyFloatVector(scores);
	DestroyMeshInfo(MI);
	DestroyRayTracer(ert);
}

TEST(C_ViewAnalysisCInterface, SphericalViewAnalysisAggregateFlat) {
	// Create Plane
	const std::vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create and allocate a new instacnce of meshinfo
	std::vector<MeshInfo>* MI;
	auto MIR = StoreMesh(
		&MI,
		plane_indices.data(),
		plane_indices.size(),
		plane_vertices.data(),
		plane_vertices.size(),
		"",
		0
	);
	ASSERT_EQ(MIR, HF::Exceptions::HF_STATUS::OK);

	// Create a new raytracer
	EmbreeRayTracer* ert;
	CreateRaytracer(MI, &ert);

	// Create Nodes
	std::vector<float> nodes = {
		0,0,1,
		0,0,2,
		0,0,3,
	};

	// Make settings
	float max_rays = 10000;
	float up_fov = 90;
	float down_fov = 90;
	float height = 1.7f;
	AGGREGATE_TYPE AT = AGGREGATE_TYPE::AVERAGE;

	std::vector<float>* scores;
	float* scores_ptr;
	int scores_size;

	// Run View Analysis
	auto result = SphereicalViewAnalysisAggregateFlat(
		ert,
		nodes.data(),
		nodes.size() / 3,
		max_rays,
		up_fov,
		down_fov,
		height,
		AT,
		&scores,
		&scores_ptr,
		&scores_size
	);
	ASSERT_EQ(result, HF::Exceptions::HF_STATUS::OK);
	ASSERT_EQ(scores->size(), scores_size);

	// print Results
	for (int i = 0; i < scores->size(); i++)
		std::cerr << (*scores)[i] << std::endl;

	// Deallocate Memory
	DestroyFloatVector(scores);
	DestroyMeshInfo(MI);
	DestroyRayTracer(ert);
}

TEST(C_ViewAnalysisCInterface, SphericalDistribute)
{
	int num_rays = 10;
	std::vector<float>* out_float;
	float* out_float_data;
	float up_fov = 90.0f;
	float down_fov = 90.0f;

	auto status = SphericalDistribute(
		&num_rays, 
		&out_float, 
		&out_float_data,
		up_fov,
		down_fov
	);
	ASSERT_EQ(status, HF::Exceptions::HF_STATUS::OK);
	
	std::cerr << "Number of rays: " << num_rays << std::endl;
	for (int i = 0; i < num_rays; i++) {
		int os = i * 3;

		std::cerr << "("
			<< out_float_data[os] << ", "
			<< out_float_data[os+1] << ", "
			<< out_float_data[os+2] << ")"
			<< std::endl;
	}

	DestroyFloatVector(out_float);
}


TEST(C_ViewAnalysisCInterface, SphericalViewAnalysisNoAggregateFlat) {
	// Create Plane
	const std::vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create and allocate a new instacnce of meshinfo
	std::vector<MeshInfo>* MI;
	auto MIR = StoreMesh(
		&MI,
		plane_indices.data(),
		plane_indices.size(),
		plane_vertices.data(),
		plane_vertices.size(),
		"",
		0
	);

	// Create a new raytracer
	EmbreeRayTracer* ert;
	CreateRaytracer(MI, &ert);

	// Create Nodes
	std::vector<float> nodes = {
		0,0,1,
		0,0,2,
		0,0,3,
	};

	// Make settings
	int max_rays = 10;
	float up_fov = 90;
	float down_fov = 90;
	float height = 1.7f;
	AGGREGATE_TYPE AT = AGGREGATE_TYPE::AVERAGE;

	std::vector<RayResult>* results;
	RayResult * results_ptr;

	// Run View Analysis
	auto result = SphericalViewAnalysisNoAggregateFlat(
		ert,
		nodes.data(),
		nodes.size() / 3,
		&max_rays,
		up_fov,
		down_fov,
		height,
		&results,
		&results_ptr
	);
	ASSERT_EQ(result, HF::Exceptions::HF_STATUS::OK);

	// print Results
	std::cerr << "Num Rays: " << max_rays << std::endl;
	for (int i = 0; i < nodes.size() / 3; i++) {
		std::cerr << "Node " << i << ": ";
		for (int k = 0; k < results->size() / 3; k++) {
			int os = max_rays * i;
			std::cerr << "(" << results_ptr[k+os].meshid
			<< ", " << results_ptr[k+os].distance << "), ";
		}
		std::cerr << std::endl;
	}

	std::cerr << std::endl;

	// Deallocate Memory
	DestroyRayResultVector(results);
	DestroyMeshInfo(MI);
	DestroyRayTracer(ert);
}

TEST(C_ViewAnalysisCInterface, SphericalViewAnalysisNoAggregate) {
	// Create Plane
	const std::vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};
	const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

	// Create and allocate a new instacnce of meshinfo
	std::vector<MeshInfo>* MI;
	auto MIR = StoreMesh(
		&MI,
		plane_indices.data(),
		plane_indices.size(),
		plane_vertices.data(),
		plane_vertices.size(),
		"",
		0
	);

	// Create a new raytracer
	EmbreeRayTracer* ert;
	CreateRaytracer(MI, &ert);

	// Create Nodes
	std::vector<Node> nodes = {
		Node(0,0,1),
		Node(0,0,2),
		Node(0,0,3),
	};

	// Make settings
	int max_rays = 10;
	float up_fov = 90;
	float down_fov = 90;
	float height = 1.7f;
	AGGREGATE_TYPE AT = AGGREGATE_TYPE::AVERAGE;

	std::vector<RayResult>* results;
	RayResult* results_ptr;

	// Run View Analysis
	auto result = SphericalViewAnalysisNoAggregate(
		ert,
		nodes.data(),
		nodes.size(),
		&max_rays,
		up_fov,
		down_fov,
		height,
		&results,
		&results_ptr
	);
	ASSERT_EQ(result, HF::Exceptions::HF_STATUS::OK);

	// print Results
	std::cerr << "Num Rays: " << max_rays << std::endl;
	for (int i = 0; i < nodes.size(); i++) {
		std::cerr << "Node " << i << ": ";
		for (int k = 0; k < results->size()/3; k++) {
			int os = max_rays * i;
			std::cerr << "(" << results_ptr[k + os].meshid
				<< ", " << results_ptr[k + os].distance << "), ";
		}
		std::cerr << std::endl;
	}

	std::cerr << std::endl;

	// Deallocate Memory
	DestroyRayResultVector(results);
	DestroyMeshInfo(MI);
	DestroyRayTracer(ert);
}
*/