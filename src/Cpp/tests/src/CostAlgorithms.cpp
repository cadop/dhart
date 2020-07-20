#include "gtest/gtest.h"
#include <robin_hood.h>
#include <graph.h>
#include <node.h>
#include <edge.h>
#include <spatialstructures_C.h>
#include <cost_algorithms.h>

using namespace HF::SpatialStructures;

namespace CostAlgorithmsTests {
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

	TEST(_CostAlgorithmsCInterface, CalculateAndStoreEnergyExpenditureWithEnergyBlob) {


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
		g.addEdge(n0, n1);	// [ -2, -6, -6 ]
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
	}
		std::vector<EdgeSet> edge_costs = CalculateEnergyExpenditure(sg);
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
		std::vector<std::vector<EdgeSet>> all_edge_costs = CalculateEnergyExpenditure(g);
	}


	TEST(_CostAlgorithms, CalculateEnergyExpenditureWithEnergyBlob) {
	}
}