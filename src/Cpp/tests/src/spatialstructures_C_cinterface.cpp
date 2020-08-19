/*!
	\file		spatialstructures_C_cinterface.cpp
	\brief		Unit test source file for functions tested from spatialstructures_C.h

	\author		TBA
	\date		19 Aug 2020
*/

#include "gtest/gtest.h"

#include "graph.h"
#include "constants.h"
#include "HFExceptions.h"

#include "spatialstructures_C.h"

namespace CInterfaceTests {
	TEST(_spatialstructures_cinterface, AlgorithmCostTitle) {
		//! [snippet_spatialstructuresC_AlgorithmCostTitle]
		// TODO example
		//! [snippet_spatialstructuresC_AlgorithmCostTitle]
	}

	TEST(_spatialstructures_cinterface, GetAllNodesFromGraph) {
		//! [snippet_spatialstructuresC_GetAllNodesFromGraph]
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		float n0[] = { 0, 0, 0 };
		float n1[] = { 0, 1, 2 };
		float n2[] = { 0, 1, 3 };

		const char* cost_type = "";

		AddEdgeFromNodes(g, n0, n1, 1, cost_type);
		AddEdgeFromNodes(g, n0, n2, 2, cost_type);
		AddEdgeFromNodes(g, n1, n0, 3, cost_type);
		AddEdgeFromNodes(g, n1, n2, 4, cost_type);
		AddEdgeFromNodes(g, n2, n0, 5, cost_type);
		AddEdgeFromNodes(g, n2, n1, 6, cost_type);

		auto out_vec = new std::vector<HF::SpatialStructures::Node>;
		HF::SpatialStructures::Node* out_data = nullptr;

		GetAllNodesFromGraph(g, &out_vec, &out_data);

		DestroyGraph(g);
		//! [snippet_spatialstructuresC_GetAllNodesFromGraph]
	}

	TEST(_spatialstructures_cinterface, GetEdgesForNode) {
		//! [snippet_spatialstructuresC_GetEdgesForNode]
		// TODO example
		//! [snippet_spatialstructuresC_GetEdgesForNode]
	}

	TEST(_spatialstructures_cinterface, GetSizeOfNodeVector) {
		//! [snippet_spatialstructuresC_GetSizeOfNodeVector]
		// Requires #include "node.h", #include <vector>

		HF::SpatialStructures::Node n0(0, 0, 0);
		HF::SpatialStructures::Node n1(0, 1, 1);
		HF::SpatialStructures::Node n2(0, 1, 2);
		HF::SpatialStructures::Node n3(1, 2, 3);

		std::vector<HF::SpatialStructures::Node>* node_vec = new std::vector<HF::SpatialStructures::Node>{ n0, n1, n2, n3 };

		int node_vec_size = -1;
		GetSizeOfNodeVector(node_vec, &node_vec_size);

		DestroyNodes(node_vec);
		//! [snippet_spatialstructuresC_GetSizeOfNodeVector]
	}

	TEST(_spatialstructures_cinterface, GetSizeOfEdgeVector) {
		//! [snippet_spatialstructuresC_GetSizeOfEdgeVector]
		// TODO example
		//! [snippet_spatialstructuresC_GetSizeOfEdgeVector]
	}

	TEST(_spatialstructures_cinterface, GetEdgeCost) {
		//! [snippet_spatialstructuresC_GetEdgeCost]
		// TODO example
		//! [snippet_spatialstructuresC_GetEdgeCost]
	}

	TEST(_spatialstructures_cinterface, AggregateCosts) {
		//! [snippet_spatialstructuresC_AggregateCosts]
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		float n0[] = { 0, 0, 0 };
		float n1[] = { 0, 1, 2 };
		float n2[] = { 0, 1, 3 };

		const char* cost_type = "";

		AddEdgeFromNodes(g, n0, n1, 1, cost_type);
		AddEdgeFromNodes(g, n0, n2, 2, cost_type);
		AddEdgeFromNodes(g, n1, n0, 3, cost_type);
		AddEdgeFromNodes(g, n1, n2, 4, cost_type);
		AddEdgeFromNodes(g, n2, n0, 5, cost_type);
		AddEdgeFromNodes(g, n2, n1, 6, cost_type);

		std::vector<float>* out_vector = nullptr;
		float* out_data = nullptr;

		int aggregation_type = 0;
		AggregateCosts(g, aggregation_type, false, cost_type, &out_vector, &out_data);

		DestroyGraph(g);
		//! [snippet_spatialstructuresC_AggregateCosts]
	}

	TEST(_spatialstructures_cinterface, CreateGraph) {
		//! [snippet_spatialstructuresC_CreateGraph]
			// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		// use Graph

		// Release memory for g after use
		DestroyGraph(g);
		//! [snippet_spatialstructuresC_CreateGraph]
	}

	TEST(_spatialstructures_cinterface, AddEdgeFromNodes) {
		//! [snippet_spatialstructuresC_AddEdgeFromNodes]
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		float n0[] = { 0, 0, 0 };
		float n1[] = { 0, 1, 2 };
		const float distance = 3;

		const char* cost_type = "";

		AddEdgeFromNodes(g, n0, n1, distance, cost_type);

		// Release memory for g after use
		DestroyGraph(g);
		//! [snippet_spatialstructuresC_AddEdgeFromNodes]
	}

	TEST(_spatialstructures_cinterface, AddEdgeFromNodeIDs) {
		//! [snippet_spatialstructuresC_AddEdgeFromNodeIDs]
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}
		const int id0 = 0;
		const int id1 = 1;
		const float distance = 3;

		const char* cost_type = "";

		AddEdgeFromNodeIDs(g, id0, id1, distance, cost_type);

		// Release memory for g after use
		DestroyGraph(g);
		//! [snippet_spatialstructuresC_AddEdgeFromNodeIDs]
	}

	TEST(_spatialstructures_cinterface, GetCSRPointers) {
		//! [snippet_spatialstructuresC_GetCSRPointers]
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		float n0[] = { 0, 0, 0 };
		float n1[] = { 0, 1, 2 };
		float n2[] = { 0, 1, 3 };

		const char* cost_type = "";

		AddEdgeFromNodes(g, n0, n1, 1, cost_type);
		AddEdgeFromNodes(g, n0, n2, 2, cost_type);
		AddEdgeFromNodes(g, n1, n0, 3, cost_type);
		AddEdgeFromNodes(g, n1, n2, 4, cost_type);
		AddEdgeFromNodes(g, n2, n0, 5, cost_type);
		AddEdgeFromNodes(g, n2, n1, 6, cost_type);

		Compress(g);

		// data = { 1, 2, 3, 4, 5, 6 }
		// r = { 0, 2, 4 }
		// c = { 1, 2, 0, 2, 0, 1 }

		// Retrieve the CSR from the graph
		HF::SpatialStructures::CSRPtrs csr;
		GetCSRPointers(g, &csr.nnz, &csr.rows, &csr.cols, &csr.data, &csr.inner_indices, &csr.outer_indices, cost_type);

		// Release memory for g after use
		DestroyGraph(g);
		//! [snippet_spatialstructuresC_GetCSRPointers]
	}

	TEST(_spatialstructures_cinterface, GetNodeID) {
		//! [snippet_spatialstructuresC_GetNodeID]
			// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		float n0[] = { 0, 0, 0 };
		float n1[] = { 0, 1, 2 };
		const float distance = 3;

		const char* cost_type = "";

		AddEdgeFromNodes(g, n0, n1, distance, cost_type);

		float point[] = { 0, 1, 2 };
		int result_id = -1;

		GetNodeID(g, point, &result_id);

		// Release memory fo
		//! [snippet_spatialstructuresC_GetNodeID]
	}

	TEST(_spatialstructures_cinterface, Compress) {
		//! [snippet_spatialstructuresC_Compress]
			// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		float n0[] = { 0, 0, 0 };
		float n1[] = { 0, 1, 2 };
		float n2[] = { 0, 1, 3 };

		const char* cost_type = "";

		AddEdgeFromNodes(g, n0, n1, 1, cost_type);
		AddEdgeFromNodes(g, n0, n2, 2, cost_type);
		AddEdgeFromNodes(g, n1, n0, 3, cost_type);
		AddEdgeFromNodes(g, n1, n2, 4, cost_type);
		AddEdgeFromNodes(g, n2, n0, 5, cost_type);
		AddEdgeFromNodes(g, n2, n1, 6, cost_type);

		Compress(g);

		// data = { 1, 2, 3, 4, 5, 6 }
		// r = { 0, 2, 4 }
		// c = { 1, 2, 0, 2, 0, 1 }

		// Release memory for g after use
		DestroyGraph(g);
		//! [snippet_spatialstructuresC_Compress]
	}

	TEST(_spatialstructures_cinterface, ClearGraph) {
		//! [snippet_spatialstructuresC_ClearGraph]
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		float n0[] = { 0, 0, 0 };
		float n1[] = { 0, 1, 2 };
		const float distance = 3;

		const char* cost_type = "";

		AddEdgeFromNodes(g, n0, n1, distance, cost_type);

		ClearGraph(g, cost_type);

		// Release memory for g after use
		DestroyGraph(g);
		//! [snippet_spatialstructuresC_ClearGraph]
	}

	TEST(_spatialstructures_cinterface, DestroyNodes) {
		//! [snippet_spatialstructuresC_DestroyNodes]
		// Requires #include "node.h", #include <vector>

		HF::SpatialStructures::Node n0(0, 0, 0);
		HF::SpatialStructures::Node n1(0, 1, 1);
		HF::SpatialStructures::Node n2(0, 1, 2);
		HF::SpatialStructures::Node n3(1, 2, 3);

		auto node_vec = new std::vector<HF::SpatialStructures::Node>{ n0, n1, n2, n3 };

		// Use node_vec

		DestroyNodes(node_vec);
		//! [snippet_spatialstructuresC_DestroyNodes]
	}

	TEST(_spatialstructures_cinterface, DestroyEdges) {
		//! [snippet_spatialstructuresC_DestroyEdges]
		// Requires #include "node.h", #include <vector>

		HF::SpatialStructures::Node n0(0, 0, 0);
		HF::SpatialStructures::Node n1(0, 1, 1);
		HF::SpatialStructures::Node n2(0, 1, 2);
		HF::SpatialStructures::Node n3(1, 2, 3);

		HF::SpatialStructures::Edge e0(n1); // parent is n0
		HF::SpatialStructures::Edge e1(n3); // parent is n2

		auto edge_vec = new std::vector<HF::SpatialStructures::Edge>{ e0, e1 };

		// Use edge_vec

		DestroyEdges(edge_vec);
		//! [snippet_spatialstructuresC_DestroyEdges]
	}

	TEST(_spatialstructures_cinterface, DestroyGraph) {
		//! [snippet_spatialstructuresC_DestroyGraph]
		// Requires #include "graph.h"

		HF::SpatialStructures::Graph* g = nullptr;

		// parameters nodes and num_nodes are unused, according to documentation
		if (CreateGraph(nullptr, -1, &g)) {
			std::cout << "Graph creation successful";
		}
		else {
			std::cout << "Graph creation failed" << std::endl;
		}

		// use Graph

		// Release memory for g after use
		DestroyGraph(g);
		//! [snippet_spatialstructuresC_DestroyGraph]
	}

	TEST(_spatialstructures_cinterface, CalculateAndStoreCrossSlope) {
		//! [snippet_spatialstructuresC_CalculateAndStoreCrossSlope]
		// Requires #include "graph.h"

		// Create 7 nodes
		HF::SpatialStructures::Node n0(0, 0, 0);
		HF::SpatialStructures::Node n1(1, 3, 5);
		HF::SpatialStructures::Node n2(3, -1, 2);
		HF::SpatialStructures::Node n3(1, 2, 1);
		HF::SpatialStructures::Node n4(4, 5, 7);
		HF::SpatialStructures::Node n5(5, 3, 2);
		HF::SpatialStructures::Node n6(-2, -5, 1);

		HF::SpatialStructures::Graph g;

		// Adding 9 edges
		g.addEdge(n0, n1);
		g.addEdge(n1, n2);
		g.addEdge(n1, n3);
		g.addEdge(n1, n4);
		g.addEdge(n2, n4);
		g.addEdge(n3, n5);
		g.addEdge(n5, n6);
		g.addEdge(n4, n6);

		// Always compress the graph after adding edges!
		g.Compress();

		// Within CalculateAndStoreCrossSlope,
		// std::vector<std::vector<IntEdge>> CostAlgorithms::CalculateAndStoreCrossSlope(Graph& g)
		// will be called, along with a call to the member function
		// void Graph::AddEdges(std::vector<std::vector<IntEdge>>& edges).
		CalculateAndStoreCrossSlope(&g);
		//! [snippet_spatialstructuresC_CalculateAndStoreCrossSlope]
	}

	TEST(_spatialstructures_cinterface, CalculateAndStoreEnergyExpenditure) {
		//! [snippet_spatialstructuresC_CalculateAndStoreEnergyExpenditure]
		// Requires #include "graph.h"

		// Create 7 nodes
		HF::SpatialStructures::Node n0(0, 0, 0);
		HF::SpatialStructures::Node n1(1, 3, 5);
		HF::SpatialStructures::Node n2(3, -1, 2);
		HF::SpatialStructures::Node n3(1, 2, 1);
		HF::SpatialStructures::Node n4(4, 5, 7);
		HF::SpatialStructures::Node n5(5, 3, 2);
		HF::SpatialStructures::Node n6(-2, -5, 1);

		HF::SpatialStructures::Graph g;

		// Adding 9 edges
		g.addEdge(n0, n1);
		g.addEdge(n1, n2);
		g.addEdge(n1, n3);
		g.addEdge(n1, n4);
		g.addEdge(n2, n4);
		g.addEdge(n3, n5);
		g.addEdge(n5, n6);
		g.addEdge(n4, n6);

		// Always compress the graph after adding edges!
		g.Compress();

		// Within CalculateAndStoreEnergyExpenditure,
		// std::vector<std::vector<EdgeSet>> CostAlgorithms::CalculateAndStoreEnergyExpenditure(Graph& g)
		// will be called, along with a call to the member function
		// void Graph::AddEdges(std::vector<std::vector<EdgeSet>>& edges).
		CalculateAndStoreEnergyExpenditure(&g);
		//! [snippet_spatialstructuresC_CalculateAndStoreEnergyExpenditure]
	}

	TEST(_spatialstructures_cinterface, AddNodeAttributes) {
		//! [snippet_spatialstructuresC_AddNodeAttributes]
		// TODO example
		//! [snippet_spatialstructuresC_AddNodeAttributes]
	}

	TEST(_spatialstructures_cinterface, GetNodeAttributes) {
		//! [snippet_spatialstructuresC_GetNodeAttributes]
		// TODO example
		//! [snippet_spatialstructuresC_GetNodeAttributes]
	}

	TEST(_spatialstructures_cinterface, DeleteScoreArray) {
		//! [snippet_spatialstructuresC_DeleteScoreArray]
		// TODO example
		//! [snippet_spatialstructuresC_DeleteScoreArray]
	}

	TEST(_spatialstructures_cinterface, ClearAttributeType) {
		//! [snippet_spatialstructuresC_ClearAttributeType]
		// TODO example
		//! [snippet_spatialstructuresC_ClearAttributeType]
	}

	TEST(_spatialstructures_cinterface, GetSizeOfGraph) {
		//! [snippet_spatialstructuresC_GetSizeOfGraph]
		// TODO example
		//! [snippet_spatialstructuresC_GetSizeOfGraph]
	}
}
