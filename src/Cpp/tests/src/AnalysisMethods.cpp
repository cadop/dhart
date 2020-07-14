#include "gtest/gtest.h"
#include <array>
#include <graph_generator.h>
#include <unique_queue.h>
#include <embree_raytracer.h>
#include <objloader.h>
#include <meshinfo.h>
#include <graph.h>
#include <edge.h>
#include <node.h>
#include <Constants.h>
#include <analysis_C.h>


#include "performance_testing.h"

using std::vector;

using HF::RayTracer::EmbreeRayTracer;
using HF::Geometry::MeshInfo;

/// <summary>
/// Create a new raytracer from a basic 10x10 plane centered on the origin.
/// </summary>
inline EmbreeRayTracer CreateRTWithPlane() {
	const vector<float> plane_vertices{
		-10.0f, 10.0f, 0.0f,
		-10.0f, -10.0f, 0.0f,
		10.0f, 10.0f, 0.0f,
		10.0f, -10.0f, 0.0f,
	};

	const vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };
	return EmbreeRayTracer(vector<MeshInfo>{MeshInfo(plane_vertices, plane_indices, 0, " ")});
}


namespace HF {

	TEST(_GraphGenerator, GraphGeneratorParallel) {
		auto mesh = Geometry::LoadMeshObjects("plane.obj", HF::Geometry::ONLY_FILE, true);
		RayTracer::EmbreeRayTracer rt(mesh);
		auto GG = GraphGenerator::GraphGenerator(rt, 0);
	
		auto g = GG.BuildNetwork(
			std::array<float, 3>{0, 0, 0.5},
			std::array<float, 3>{0.02f, 0.02f, 0.02f},
			1000,
			1,
			1,
			1,
			5,
			1
		);
		printf("Graph size %i\n", g.size());
		g.Compress();
		ASSERT_GT(g.size(), 0);
	}


	TEST(_GraphGenerator, DuplicateNodes) {
		auto mesh = Geometry::LoadMeshObjects("energy_blob.obj");

		RayTracer::EmbreeRayTracer rt(mesh);
		auto GG = GraphGenerator::GraphGenerator(rt, 0);

		// Generate the graph 
		auto g = GG.BuildNetwork(
			std::array<float, 3>{-22.4280376, -12.856843, 5.4826779},
			std::array<float, 3>{10.0f, 10.0f, 70.0f},
			50,
			10,
			40,
			10,
			1,
			2
		);

		ASSERT_GT(g.size(), 0);
		// Assert that the distance from this node to every other node in the graph is > than rounding precision
		const auto nodes = g.Nodes();
		for (const auto& node : nodes) {
			
			int close_to_nodes = 0;
			for (const auto& node2 : nodes)
			{
				// Check if this node closer to node2 than rounding_precision would allow
				if (node.distanceTo(node2) < HF::SpatialStructures::ROUNDING_PRECISION)
				{
					close_to_nodes++;
				}
				// If it's closer to two nodes (itself and one more) then it's not being rounded
				ASSERT_LT(close_to_nodes, 2);
			}
		}
	}

	TEST(_GraphGenerator, GraphGeneratorSingle) {
		auto mesh = Geometry::LoadMeshObjects("plane.obj", HF::Geometry::ONLY_FILE, true);
		RayTracer::EmbreeRayTracer rt(mesh);
		auto GG = GraphGenerator::GraphGenerator(rt, 0);

		auto g = GG.BuildNetwork(
			std::array<float, 3>{0, 0, 0.5},
			std::array<float, 3>{0.02f, 0.02f, 0.02f},
			1000,
			1,
			1,
			1,
			5,
			1,
			1
		);
		printf("Graph size %i\n", g.size());
		g.Compress();
		ASSERT_GT(g.size(), 0);
	}

	TEST(_UniqueQueue, BlockRepeats) {
		HF::GraphGenerator::UniqueQueue q;
		SpatialStructures::Node n1{ 1,2,3 };
		EXPECT_TRUE(q.push(n1));
		EXPECT_FALSE(q.push(n1));
	}

	TEST(_UniqueQueue, Size) {
		HF::GraphGenerator::UniqueQueue q;
		SpatialStructures::Node n1{ 1,2,3 };
		q.push(n1);
		ASSERT_EQ(q.size(), 1);
	}
	TEST(_UniqueQueue, Empty) {
		HF::GraphGenerator::UniqueQueue q;
		SpatialStructures::Node n1{ 1,2,3 };
		ASSERT_TRUE(q.empty());
		q.push(n1);
		ASSERT_FALSE(q.empty());
	}
}

namespace CInterfaceTests {
	TEST(_AnalysisCInterface, GenerateGraph) {
		// Requires #include "analysis_C.h", #include "embree_raytracer.h", #include "graph.h"

		// Create a container of coordinates
		std::vector<std::array<float, 3>> directions = {
			{0, 0, 1},
			{0, 1, 0},
			{1, 0, 0},
			{-1, 0, 0},
			{0, -1, 0},
			{0, 0, -1},
		};

		// Create the EmbreeRayTracer
		auto ert = HF::RayTracer::EmbreeRayTracer(directions);

		// Have a pointer to Graph ready
		HF::SpatialStructures::Graph* g = nullptr;

		// Prepare parameters
		float start[] = { 0, 0, 0 };
		float spacing[] = { 1, 1, 1 };
		const int max_nodes = 6;
		const float up_step = 1.5;
		const float up_slope = 1.0;
		const float down_step = 2.0;
		const float down_slope = 0.5;
		const int maximum_step_connections = 2;
		const int cores = 4;
		
		if (GenerateGraph(&ert, start, spacing, max_nodes, up_step, up_slope, down_step, down_slope, maximum_step_connections, cores, &g)) {
			std::cout << "GenerateGraph successful" << std::endl;
		}
		else {
			std::cout << "GenerateGraph failed" << std::endl;
		}

		// Free memory resources once finished with Graph		
		if (g) {
			delete g;
			g = nullptr;
		}
	}
}

namespace CInterfaceTests {
	TEST(_CInterfaceUtils, ConvertRawFloatArrayToPoints) {
		// Requires #include "cinterface_utils.h"

		float raw_array[] = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
		const int size = 9;

		std::vector<std::array<float, 3>> points = ConvertRawFloatArrayToPoints(raw_array, size);
	}

	TEST(_CInterfaceUtils, ConvertRawIntArrayToPoints) {
		// Requires #include "cinterface_utils.h"

		int raw_array[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
		const int size = 9;

		std::vector<std::array<int, 3>> points = ConvertRawIntArrayToPoints(raw_array, size);
	}

	TEST(_CInterfaceUtils, DestroyFloatVector) {
		// Requires #include "cinterface_utils.h"

		std::vector<float>* vec = new std::vector<float>{ 3, 6, 9 };

		// operator delete called on vec within DestroyFloatVector 
		if (DestroyFloatVector(vec)) {
			std::cout << "DestroyFloatVector was successful" << std::endl;
			vec = nullptr;
		}
		else {
			std::cout << "DestroyFloatVector unsuccessful" << std::endl;
		}
	}

	TEST(_CInterfaceUtils, DeleteRawPtr) {
		// Requires #include "cinterface_utils.h"

		std::vector<int>* vec = new std::vector<int>{ 6, 3, 1, 0, 5, 0 };

		DeleteRawPtr<std::vector<int>>(vec);

		// At this point here, we assume DeleteRawPtr was successful.
		// We set vec to nullptr to eliminate a dangling pointer.
		if (vec) {
			vec = nullptr;
		}
	}

}

/*!
	\brief Evaluates how quickly the graph generator can create nodes on a flat plane.
*/
TEST(Performance, GraphGenerator) {

	// Each element here will be evaluated in seperate trial
	vector<int> max_nodes_list = { 
		10,
		100, 
		500,
		1000,
		5000, 
		10000, 
	};

	// Setup trial arrays
	const int num_trials = max_nodes_list.size();
	vector<StopWatch> watches(num_trials);	
	vector<int> nodes_generated(num_trials);	
	
	// Set Graph generator settings
	auto ray_tracer = CreateRTWithPlane();
	std::array<float, 3> start{ 0,0,0 };
	std::array<float, 3> spacing{ 0.05,0.05,0.05 };
	float up_step = 1;
	float down_step = 1;
	float up_slope = 30;
	float down_slope = 30;
	int max_step_connections = 1;


	// Run Trials and record results
	for (int i = 0; i < num_trials; i++) {
		auto& watch = watches[i];
		int max_nodes = max_nodes_list[i];

		watch.StartClock();
		auto GG = HF::GraphGenerator::GraphGenerator(ray_tracer, 0, 0);
		auto graph = GG.BuildNetwork(
			start,
			spacing, 
			max_nodes,
			up_step,
			up_slope,
			down_step,
			down_slope,
			max_step_connections
		);
		watch.StopClock();

		nodes_generated[i] = graph.size();
	}

	// Print Results
	PrintTrials(watches, nodes_generated, "Nodes");
}