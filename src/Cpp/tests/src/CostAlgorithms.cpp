#include "gtest/gtest.h"
#include <robin_hood.h>
#include <graph.h>
#include <node.h>
#include <edge.h>
#include <spatialstructures_C.h>
#include <cost_algorithms.h>

using namespace HF::SpatialStructures;

namespace CostAlgorithmsTests {
	TEST(_CostAlgorithms, CalculateCrossSlopeSubgraph) {
		using HF::SpatialStructures::CostAlgorithms::CalculateCrossSlope;
		
		// Create 7 nodes
		Node n0(2, 6, 6);
		Node n1(0, 0, 0);
		Node n2(-5, 5, 4);
		Node n3(-1, 1, 1);
		Node n4(2, 2, 2);
		Node n5(5, 3, 2);
		Node n6(-2, -5, 1);

		Graph g;
		
		// Adding 9 edges
		g.addEdge(n0, n1);	// [ -2, -6, -6 ]
		g.addEdge(n1, n2);	// [ -5,  5,  4 ]
		g.addEdge(n1, n3);  // [ -1,  1,  1 ]
		g.addEdge(n1, n4);  // [  2,  2,  2 ]
		g.addEdge(n2, n4);  // [ -9, -3, -2 ]
		g.addEdge(n3, n5);  // [ -6,  2,  1 ]
		g.addEdge(n5, n6);  // [ -7, -8, -1 ]
		g.addEdge(n4, n6);  // [ -6, -7, -1 ]
	
		// Always compress the graph after adding edges!
		g.Compress();

		// Retrieve a subgraph of your choice, provide a parent node or parent node ID.
		Subgraph sg = g.GetSubgraph(n1);

		// Get a container of vector<IntEdge>, ordered by parent ID.
		// These consist of alternate edge costs for Subgraph sg.
		std::vector<IntEdge> edge_costs = CalculateCrossSlope(sg);
	}

	TEST(_CostAlgorithms, CalculateCrossSlopeAll) {
		using HF::SpatialStructures::CostAlgorithms::CalculateCrossSlope;

		// Create 7 nodes
		Node n0(2, 6, 6);
		Node n1(0, 0, 0);
		Node n2(-5, 5, 4);
		Node n3(-1, 1, 1);
		Node n4(2, 2, 2);
		Node n5(5, 3, 2);
		Node n6(-2, -5, 1);

		Graph g;

		// Adding 9 edges
		g.addEdge(n0, n1);	// [ -2, -6, -6 ]
		g.addEdge(n1, n2);	// [ -5,  5,  4 ]
		g.addEdge(n1, n3);  // [ -1,  1,  1 ]
		g.addEdge(n1, n4);  // [  2,  2,  2 ]
		g.addEdge(n2, n4);  // [ -9, -3, -2 ]
		g.addEdge(n3, n5);  // [ -6,  2,  1 ]
		g.addEdge(n5, n6);  // [ -7, -8, -1 ]
		g.addEdge(n4, n6);  // [ -6, -7, -1 ]

		// Always compress the graph after adding edges!
		g.Compress();
		
		// Get a container of vector<IntEdge>, ordered by parent ID.
		// These consist of alternate edge costs for all subgraphs in g.
		std::vector<std::vector<IntEdge>> all_edge_costs = CalculateCrossSlope(g);
	}

	/*
	TEST(_CostAlgorithms, CalculateCrossSlopeWithEnergyBlob) {
		///
		///	Cannot run this test until GraphGenerator is fixed.
		///
		using HF::Geometry::LoadMeshObjects;
		using HF::RayTracer::EmbreeRayTracer;
		using HF::GraphGenerator::GraphGenerator;
		using HF::SpatialStructures::Graph;

		std::string file_path = "energy_blob.obj";
		bool z_up = true; // LoadMeshObjects makes that param true by default anyway.
		std::vector<HF::Geometry::MeshInfo> mesh = LoadMeshObjects(file_path, HF::Geometry::GROUP_METHOD::ONLY_FILE, z_up);
		mesh[0].PerformRotation(90, 0, 0);

		EmbreeRayTracer ray_tracer(mesh);
		GraphGenerator graph_generator = GraphGenerator(ray_tracer, 0);

		std::array<float, 3> starting_position = { -22.4280376, -12.856843,  5.4826779 };
		std::array<float, 3> spacing = { 10, 10, 70 };
		auto upstep = 10;
		auto downstep = 10;
		auto up_slope = 40;
		auto down_slope = 1;
		auto max_connections_out = 2;
		auto max_nodes = 50;

		auto core_count = 1;

		Graph g = graph_generator.BuildNetwork(starting_position,
			spacing, max_nodes, upstep, up_slope,
			downstep, down_slope, core_count);

		g.Compress();

		std::vector<IntEdge> int_edge = HF::SpatialStructures::CostAlgorithms::CalculateCrossSlope(g);

		CSRPtrs csr = g.GetCSRPointers();
		auto data = csr.data;
		auto col = csr.inner_indices;
		int i = 0;
		for (int parent_id = 0; parent_id < csr.rows; parent_id++) {
			float* row_begin = csr.row_begin(parent_id);
			float* row_curr = row_begin;
			float* row_end = csr.row_end(parent_id);


			while (row_curr < row_end) {
				Node parent = g.NodeFromID(parent_id);
				Node child = g.NodeFromID(col[i]);

				std::cout << "parent ID # " << parent_id << "with pos "
				          << parent.getArray() << " has child ID # " << col[i]
					      << " at pos " << child.getArray() << " with edge value "
						  << data[i] << std::endl;
				++row_curr;
				++i;
			}
		}

	}
	*/
}

namespace CostAlgorithmsTests {
	TEST(_CostAlgorithms, to_radians) {
		// Requires #include "cost_algorithms.h"

		// for brevity
		using HF::SpatialStructures::CostAlgorithms::to_radians;

		double in_degrees = 360.0;
		double in_radians = to_radians(in_degrees); // 6.2831853.., about 2 * M_PI
	}

	TEST(_CostAlgorithms, to_degrees) {
		// Requires #include "cost_algorithms.h"

		// for brevity
		using HF::SpatialStructures::CostAlgorithms::to_degrees;

		// The macro M_PI requires
		// #define _USE_MATH_DEFINES
		// #include <cmath>
		//
		// Use M_PI in place of approximate_pi when the value pi is needed.
		double approximate_pi = 3.14159;
		double in_radians = 2 * approximate_pi;

		double in_degrees = to_degrees(in_radians);	
		// 359.99969592100859, about 360.0 degrees
	}

	TEST(_CostAlgorithms, euclidean_distance) {
		// Requires #include "cost_algorithms.h"

		// for brevity
		using HF::SpatialStructures::CostAlgorithms::euclidean_distance;

		// 2D coordinates, { x, y }
		float pos_a[] = { 0, 0 };
		float pos_b[] = { 4, 3 };

		// There are two template parameters for euclidean_distance:
		// - dimension (2 for 2D, 3 for 3D, etc)
		// - float_precision (defaults to float if unspecified)
		float distance = euclidean_distance<2>(pos_a, pos_b);	// 5.00000
	}

	TEST(_CostAlgorithms, dot_product) {
		// Requires #include "cost_algorithms.h"

		// for brevity
		using HF::SpatialStructures::CostAlgorithms::dot_product;

		// components of a 2D vector, { v_x, v_y }
		float vec_u[] = { 4, 0 };
		float vec_v[] = { 0, 4 };

		// There are two template parameters for dot_product:
		// - dimension (2 for 2D, 3 for 3D, etc)
		// - float_precision (defaults to float if unspecified)
		float dot_prod = dot_product<2>(vec_u, vec_v);	// 0.00000

		// vec_u and vec_v are perpendicular.
	}

	TEST(_CostAlgorithms, is_perpendicular) {
		// Requires #include "cost_algorithms.h"

		// for brevity
		using HF::SpatialStructures::CostAlgorithms::is_perpendicular;

		// components of a 2D vector, { v_x, v_y }
		float vec_u[] = { 4, 0 };
		float vec_v[] = { 0, 4 };

		// There are two template parameters for is_perpendicular:
		// - dimension (2 for 2D, 3 for 3D, etc)
		// - float_precision (defaults to float if unspecified)
		bool is_perp = is_perpendicular<2>(vec_u, vec_v);	// true

		// vec_u and vec_v are perpendicular.
	}

	TEST(_CostAlgorithms, GetPerpendicularEdges) {
		// Requires #include "cost_algorithms.h"

		// for brevity
		using HF::SpatialStructures::CostAlgorithms::GetPerpendicularEdges;

		// Create 7 nodes
		Node n0(2, 6, 6);
		Node n1(0, 0, 0);
		Node n2(-5, 5, 4);
		Node n3(-1, 1, 1);
		Node n4(2, 2, 2);
		Node n5(5, 3, 2);
		Node n6(-2, -5, 1);

		Graph g;

		// Adding 9 edges
		g.addEdge(n0, n1);	// [ -2, -6, -6 ]
		g.addEdge(n1, n2);	// [ -5,  5,  4 ]
		g.addEdge(n1, n3);  // [ -1,  1,  1 ]
		g.addEdge(n1, n4);  // [  2,  2,  2 ]
		g.addEdge(n2, n4);  // [ -9, -3, -2 ]
		g.addEdge(n3, n5);  // [ -6,  2,  1 ]
		g.addEdge(n5, n6);  // [ -7, -8, -1 ]
		g.addEdge(n4, n6);  // [ -6, -7, -1 ]

		// Always compress the graph after adding edges!
		g.Compress();
		
		// We will find all edges in g that are perpendicular to the edge (vector)
		// formed by n1 and n4 -- they will be stored in perp_edges.
		// Note that we have specifically chosen to check for 2D perpendicularity,
		// as seen by the template parameter, despite these nodes being 3D.
		auto perp_edges = GetPerpendicularEdges<2>(g.GetSubgraph(n1), n4);

		// The edge formed by n1 and n4 is perpendicular to
		// - the edge formed by n1 and n2
		// - the edge formed by n1 and n3
	}

}

namespace CInterfaceTests {
	TEST(_CostAlgorithmsCInterface, CalculateAndStoreCrossSlope) {
		// Create 7 nodes
		Node n0(2, 6, 6);
		Node n1(0, 0, 0);
		Node n2(-5, 5, 4);
		Node n3(-1, 1, 1);
		Node n4(2, 2, 2);
		Node n5(5, 3, 2);
		Node n6(-2, -5, 1);

		Graph g;

		// Adding 9 edges
		g.addEdge(n0, n1);	// [ -2, -6, -6 ]
		g.addEdge(n1, n2);	// [ -5,  5,  4 ]
		g.addEdge(n1, n3);  // [ -1,  1,  1 ]
		g.addEdge(n1, n4);  // [  2,  2,  2 ]
		g.addEdge(n2, n4);  // [ -9, -3, -2 ]
		g.addEdge(n3, n5);  // [ -6,  2,  1 ]
		g.addEdge(n5, n6);  // [ -7, -8, -1 ]
		g.addEdge(n4, n6);  // [ -6, -7, -1 ]

		// Always compress the graph after adding edges!
		g.Compress();

		// Within CalculateAndStoreCrossSlope,
		// std::vector<std::vector<IntEdge>> CostAlgorithms::CalculateAndStoreCrossSlope(Graph& g)
		// will be called, along with a call to the member function
		// void Graph::AddEdges(std::vector<std::vector<IntEdge>>& edges).
		CalculateAndStoreCrossSlope(&g);
	}

}

	TEST(_CostAlgorithms, Slope)
	{
		Node n1(1.5,1.5,1);
		Node n2(2.5, 2.5, 0);

		float slope = HF::SpatialStructures::CostAlgorithms::CalculateSlope(n1, n2);
		ASSERT_NEAR(slope, -35.2, 0.1);

		Node n3(0, 0, 0);
		Node n4(0, 0, 1);

		slope = HF::SpatialStructures::CostAlgorithms::CalculateSlope(n3, n4);
		ASSERT_NEAR(slope, 90, 0.001);

		Node n5(0, 0, 0);
		Node n6(1, 0, 1);

		slope = HF::SpatialStructures::CostAlgorithms::CalculateSlope(n5, n6);
		ASSERT_NEAR(slope, 45, 0.001);

		Node n7(0, 0, 0);
		Node n8(1, 0, -1);

		slope = HF::SpatialStructures::CostAlgorithms::CalculateSlope(n7, n8);
		ASSERT_NEAR(slope, -45, 0.001);

		// Give the same node twice to ensure that
		// zero is returned instead of nan.
		Node nan_check(0, 0, 0);

		ASSERT_EQ(HF::SpatialStructures::CostAlgorithms::CalculateSlope(nan_check, nan_check), 0.0);
	}


namespace CInterfaceTests {

	TEST(_CostAlgorithmsCInterface, CalculateAndStoreEnergyExpenditureWithEnergyBlob) {
	}

	TEST(_CostAlgorithms, CalculateEnergyExpenditureSubgraph) {
		// For brevity
		using HF::SpatialStructures::CostAlgorithms::CalculateEnergyExpenditure;

		// Create 7 nodes
		Node n1(0, 0, 0);
		Node n2(-5, 5, 4);
		Node n4(2, 2, 2);
		Node n5(5, 3, 2);
		Node n3(-1, 1, 1);

		Node n6(-2, -5, 1);
		Graph g;

		// Adding 9 edges
		g.addEdge(n1, n2);	// [ -5,  5,  4 ]
		g.addEdge(n1, n1);	// [ -2, -6, -6 ]
		g.addEdge(n1, n4);  // [  2,  2,  2 ]
		g.addEdge(n1, n3);  // [ -1,  1,  1 ]
		g.addEdge(n3, n5);  // [ -6,  2,  1 ]
		g.addEdge(n2, n4);  // [ -9, -3, -2 ]
		g.addEdge(n5, n6);  // [ -7, -8, -1 ]
		g.addEdge(n4, n6);  // [ -6, -7, -1 ]

		// Always compress the graph after adding edges!
		g.Compress();

		Subgraph sg = g.GetSubgraph(n1);
		// Retrieve a subgraph of your choice, provide a parent node or parent node ID.
		// Get a container of vector<EdgeSet>, ordered by parent ID.

		// These consist of alternate edge costs for Subgraph sg.
		EdgeSet edge_costs = CalculateEnergyExpenditure(sg);
	}
	TEST(_CostAlgorithms, CalculateEnergyExpenditureAll) {
		// for brevity


		using HF::SpatialStructures::CostAlgorithms::CalculateEnergyExpenditure;
		Node n0(2, 6, 6);
		// Create 7 nodes
		Node n1(0, 0, 0);
		Node n4(2, 2, 2);
		Node n3(-1, 1, 1);
		Node n2(-5, 5, 4);
		Node n6(-2, -5, 1);
		Node n5(5, 3, 2);

		Graph g;

		g.addEdge(n0, n1);	// [ -2, -6, -6 ]
		// Adding 9 edges
		g.addEdge(n1, n2);	// [ -5,  5,  4 ]
		g.addEdge(n1, n3);  // [ -1,  1,  1 ]
		g.addEdge(n1, n4);  // [  2,  2,  2 ]
		g.addEdge(n3, n5);  // [ -6,  2,  1 ]
		g.addEdge(n2, n4);  // [ -9, -3, -2 ]
		g.addEdge(n5, n6);  // [ -7, -8, -1 ]
		g.addEdge(n4, n6);  // [ -6, -7, -1 ]

		g.Compress();
		// Always compress the graph after adding edges!

		// These consist of alternate edge costs for all subgraphs in g.
		// Get a container of vector<EdgeSet>, ordered by parent ID.
		auto all_edge_costs = CalculateEnergyExpenditure(g);
	}


	TEST(_CostAlgorithms, CalculateEnergyExpenditureWithEnergyBlob) {
	}
}
