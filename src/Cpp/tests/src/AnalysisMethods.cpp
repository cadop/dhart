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

namespace HF {

	TEST(_GraphGenerator, RunGraphGenerator) {
		auto mesh = Geometry::LoadMeshObjects("plane.obj");
		RayTracer::EmbreeRayTracer rt(mesh);
		auto GG = AnalysisMethods::GraphGenerator(rt, 0);
	
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
		HF::AnalysisMethods::UniqueQueue q;
		SpatialStructures::Node n1{ 1,2,3 };
		EXPECT_TRUE(q.push(n1));
		EXPECT_FALSE(q.push(n1));
	}

	TEST(_UniqueQueue, Size) {
		HF::AnalysisMethods::UniqueQueue q;
		SpatialStructures::Node n1{ 1,2,3 };
		q.push(n1);
		ASSERT_EQ(q.size(), 1);
	}
	TEST(_UniqueQueue, Empty) {
		HF::AnalysisMethods::UniqueQueue q;
		SpatialStructures::Node n1{ 1,2,3 };
		ASSERT_TRUE(q.empty());
		q.push(n1);
		ASSERT_FALSE(q.empty());
	}
}