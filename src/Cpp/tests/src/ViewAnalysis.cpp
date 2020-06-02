#include "gtest/gtest.h"

#include <gtest/gtest.h>
#include <array>
#include <string>

#include <graph_generator.h>
#include <embree_raytracer.h>
#include <view_analysis.h>
#include <objloader.h>
#include <meshinfo.h>
#include <graph.h>
#include <edge.h>
#include <node.h>

using namespace HF::AnalysisMethods;
using HF::SpatialStructures::Graph;
using HF::SpatialStructures::Node;
using HF::RayTracer::EmbreeRayTracer;
using HF::Geometry::MeshInfo;
using namespace HF::Geometry;

// A flat plane with a wall perpendicular to the y axis 
std::string big_teapot_path = "big_teapot.obj"; 

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
