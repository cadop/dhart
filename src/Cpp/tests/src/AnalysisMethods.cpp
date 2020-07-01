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

#include "analysis_C.h"
#include "graph.h"

namespace HF {

	TEST(_GraphGenerator, RunGraphGenerator) {
		auto mesh = Geometry::LoadMeshObjects("plane.obj");
		RayTracer::EmbreeRayTracer rt(mesh);
		auto GG = GraphGenerator::GraphGenerator(rt, 0);
	
		auto g = GG.BuildNetwork(
			std::array<float, 3>{0, 0, 0.5},
			std::array<float, 3>{0.02f, 0.02f, 0.02f},
			1000,
			1,
			1,
			1,
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