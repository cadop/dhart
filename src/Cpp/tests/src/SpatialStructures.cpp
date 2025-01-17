#include "gtest/gtest.h"
#include <robin_hood.h>
#include <graph.h>
#include <node.h>
#include <edge.h>
#include <constants.h>
#include <HFExceptions.h>
#include <spatialstructures_C.h>


using namespace HF::SpatialStructures;
using std::vector;
using std::string;
using HF::Exceptions::HF_STATUS;

namespace GraphTests {
    TEST(_Graph, Creation) {
        HF::SpatialStructures::Graph g;
        ASSERT_EQ(g.size(), 0);
    }

    TEST(_Graph, AddNodeTest) {
        HF::SpatialStructures::Graph g;
        HF::SpatialStructures::Node N1(39,39,39);
        HF::SpatialStructures::Node N2(54,54,54);

        g.addEdge(N1, N2);
        ASSERT_TRUE(g.hasKey(N1));
    }

    TEST(_Graph, HasKeyFailTest) {
        HF::SpatialStructures::Graph g;

        HF::SpatialStructures::Node N1(39,39,39);
        HF::SpatialStructures::Node N2(54,54,54);

        g.addEdge(N1, N2);
        g.Compress();
        EXPECT_TRUE(g.size() == 2);
        ASSERT_TRUE(g.hasKey(N2));
        ASSERT_TRUE(g.hasKey(N1));
    }

    TEST(_Graph, GetUndirectedEdges) {
        HF::SpatialStructures::Graph g;

        HF::SpatialStructures::Node N1(39,39,39);
        HF::SpatialStructures::Node N2(54,54,54);

        g.addEdge(N1, N2);
        g.Compress();

        ASSERT_EQ(g.GetUndirectedEdges(N2).size(), 1);
        ASSERT_EQ(g[N2].size(), 0);
    }

	TEST(_Graph, GetUndirectedEdgesMulti) {
		HF::SpatialStructures::Graph g;

		HF::SpatialStructures::Node N1(39, 39, 39);
		HF::SpatialStructures::Node N2(54, 54, 54);

		g.addEdge(N1, N2, 100);
		g.Compress();
		g.addEdge(N1, N2, 200, "AltCost");

		// Assert that getting it for this cost gets the proper values
		const auto undirected_edges = g.GetUndirectedEdges(N2, "AltCost");
		ASSERT_EQ(undirected_edges[0].score, 200);
		ASSERT_EQ(undirected_edges[0].child, N1);
	}


    TEST(_Graph, HasEdgeTrue) {
        HF::SpatialStructures::Graph g;

        HF::SpatialStructures::Node N1(39,39,39);
        HF::SpatialStructures::Node N2(54,54,54);

        g.addEdge(N1, N2);
        g.Compress();
        ASSERT_TRUE(g.HasEdge(N1, N2));
    }

    TEST(_Graph, HasEdgeFalse) {
        HF::SpatialStructures::Graph g;

        HF::SpatialStructures::Node N1(39,39,39);
        HF::SpatialStructures::Node N2(54,54,54);

        g.addEdge(N1, N2);
        g.Compress();
        ASSERT_FALSE(g.HasEdge(N1, N1));
    }

	TEST(_Graph, HasEdgeMulti) {

		string alt_cost = "alternate";

		Graph g;
		g.Compress();
		g.addEdge(1, 2, 39);
		g.addEdge(1, 2, 54, alt_cost);


		ASSERT_TRUE(g.HasEdge(1, 2, false, alt_cost));
		ASSERT_TRUE(g.HasEdge(2, 1, true, alt_cost));
		ASSERT_FALSE(g.HasEdge(1, 11, false, alt_cost));
		ASSERT_FALSE(g.HasEdge(1, 2, false, "NotSeenCost"));
	}

	inline Graph CreateTestAggregateGraph() {
		HF::SpatialStructures::Graph g;

		HF::SpatialStructures::Node N1(39, 39, 39);
		HF::SpatialStructures::Node N2(54, 54, 54);
		HF::SpatialStructures::Node N3(100, 100, 100);

		g.addEdge(N1, N2, 30);
		g.addEdge(N2, N1, 15);
		g.addEdge(N1, N3, 45);

		g.Compress();
		return g;
	}

	TEST(_Graph, AggregateCosts_Sum) {
	
		auto g = CreateTestAggregateGraph();

		auto sums = g.AggregateGraph(COST_AGGREGATE::SUM);

		EXPECT_EQ(sums[0], 75);
		EXPECT_EQ(sums[1], 15);
		EXPECT_EQ(sums[2], 0);
	}

	TEST(_Graph, AggregateCosts_Average) {
		auto g = CreateTestAggregateGraph();

		auto averages = g.AggregateGraph(COST_AGGREGATE::AVERAGE);

		EXPECT_NEAR(averages[0], 37.5f, 0.0001f);
		EXPECT_NEAR(averages[1], 15.0f, 0.0001f);
		EXPECT_NEAR(averages[2], 0.0f, 0.0001f);
	}

	TEST(_Graph, AggregateCosts_Count) {
		auto g = CreateTestAggregateGraph();

		auto counts = g.AggregateGraph(COST_AGGREGATE::COUNT);

		EXPECT_EQ(counts[0], 2);
		EXPECT_EQ(counts[1], 1);
		EXPECT_EQ(counts[2], 0);
	}

	TEST(_Graph, AggregateCosts_Undirected_Sum) {

		auto g = CreateTestAggregateGraph();

		auto sums = g.AggregateGraph(COST_AGGREGATE::SUM, false);

		EXPECT_EQ(sums[0], 90);
		EXPECT_EQ(sums[1], 45);
		EXPECT_EQ(sums[2], 45);
	}

	TEST(_Graph, AggregateCosts_Undirected_Average) {
		auto g = CreateTestAggregateGraph();

		auto averages = g.AggregateGraph(COST_AGGREGATE::AVERAGE, false);

		EXPECT_NEAR(averages[0], 30.0f, 0.0001f);
		EXPECT_NEAR(averages[1], 22.5f, 0.0001f);
		EXPECT_NEAR(averages[2], 45.0f, 0.0001f);
	}

	TEST(_Graph, AggregateCosts_Undirected_Count) {
		auto g = CreateTestAggregateGraph();

		auto counts = g.AggregateGraph(COST_AGGREGATE::COUNT, false);

		EXPECT_EQ(counts[0], 3);
		EXPECT_EQ(counts[1], 2);
		EXPECT_EQ(counts[2], 1);
	}
	TEST(_Graph, AggregateCostsMulti) {
		HF::SpatialStructures::Graph g;

		HF::SpatialStructures::Node N1(39, 39, 39);
		HF::SpatialStructures::Node N2(54, 54, 54);

		g.Compress();
		g.addEdge(N1, N2, 30);
		g.addEdge(N1, N2, 39, "TestCost");
	
		ASSERT_EQ(g.AggregateGraph(COST_AGGREGATE::SUM, true, "TestCost")[0], 39);
		ASSERT_EQ(g.AggregateGraph(COST_AGGREGATE::SUM, true, "TestCost")[1], 0);
	}

	TEST(_Graph, GetCostTypes) {
		// Create the graph in some nodes
		Graph g;
		Node N1(39, 39, 39);
		Node N2(54, 54, 54);

		// Add an edge to the graph
		g.Compress();
		g.addEdge(N1, N2, 30);
		
		// First assert that this can be called before costs have been added
		auto costs_before_added = g.GetCostTypes();
		ASSERT_EQ(costs_before_added.size(), 0);
		
		// Then add an edge with an alternate cost type to effectively create this new cost
		g.addEdge(N1, N2, 39, "TestCost");
		
		// Get cost types from the graph
		const auto costs = g.GetCostTypes();

		// Check that the size of the returned costtypes is what we think it should be
		ASSERT_EQ(costs.size(), 1);

		// See if we can find the cost in the set of returned cost types.
		ASSERT_TRUE(std::find(costs.begin(), costs.end(), "TestCost") != costs.end());
		
		// See that we don't find a cost that doesn't exist
		ASSERT_TRUE(std::find(costs.begin(), costs.end(), "CostThatDoesn'tExist") == costs.end());
	}

	TEST(_Graph, GetEdgeCosts) {
		// Create the graph in some nodes
		Graph g;

		// Add an edge to the graph
		g.Compress();
		g.addEdge(1, 2, 30.0f);
		g.addEdge(1, 3, 11.0f);
		g.addEdge(1, 4, 22.0f);
		g.addEdge(2, 1, 33.0f);
		g.addEdge(3, 2, 34.0f);
		g.addEdge(3, 4, 35.0f);

		const std::string cost_name = "TestCost";
		// First assert that this can be called before costs have been added
		auto costs_before_added = g.GetEdgeCosts(cost_name);
		ASSERT_EQ(costs_before_added.size(), 0);

		// Then add an edge with an alternate cost type to effectively create this new cost
		g.addEdge(1, 2, 39.0f, cost_name);
		g.addEdge(1, 3, 11.0f, cost_name);
		g.addEdge(1, 4, 22.0f, cost_name);
		g.addEdge(2, 1, 33.0f, cost_name);
		g.addEdge(3, 2, 34.0f, cost_name);
		g.addEdge(3, 4, 35.0f, cost_name);

		// Test getting all edge costs of type cost_name
		auto costs_after_added = g.GetEdgeCosts(cost_name);

		vector<float> expected_costs = { 39.0f, 11.0f, 22.0f, 33.0f, 34.0f, 35.0f };
		int expected_costs_size = expected_costs.size();

		ASSERT_EQ(costs_after_added.size(), expected_costs.size());

		for (int i = 0; i < expected_costs_size; i++)
		{
			ASSERT_EQ(costs_after_added[i], expected_costs[i]);
		}
		
		// Test getting edge costs of subset of edges
		vector<int> ids = { 1, 2, 3, 2, 3, 4 };
		auto specific_costs_after_added = g.GetEdgeCostsFromNodeIDs(ids, cost_name);

		vector<float> specific_expected_costs = { 39, 34, 35 };
		int expected_size = specific_expected_costs.size();

		ASSERT_EQ(specific_costs_after_added.size(), expected_size);

		for (int i = 0; i < expected_size; i++)
		{
			ASSERT_EQ(specific_costs_after_added[i], specific_expected_costs[i]);
		}
	}

	TEST(_Graph, CountNumberOfEdges) {
		HF::SpatialStructures::Graph g;
		g.Compress();
		HF::SpatialStructures::Node N1(1, 1, 2);
		HF::SpatialStructures::Node N2(2, 3, 4, 5);
		HF::SpatialStructures::Node N3(3, 10, 2, 10);
		g.addEdge(N1, N2, 1);
		g.addEdge(N2, N3, 1);

		const std::string cost_name = "TestCost";

		g.addEdge(N1, N2, 30, cost_name);
		g.addEdge(N2, N3, 20, cost_name);

		auto number_of_edges_after = g.CountEdges(cost_name);
		ASSERT_EQ(number_of_edges_after, 2);
	}
    TEST(_Graph, SizeEqualsNumberOfNodes) {
        HF::SpatialStructures::Graph g;

        HF::SpatialStructures::Node N1(1, 1, 2);
        HF::SpatialStructures::Node N2(2, 3, 4, 5);

        g.addEdge(N1, N2);
        auto nodes = g.Nodes();
        assert(g.size() == nodes.size());
    }

	TEST(_Graph, SizeEqualsNumberOfInts) {
		HF::SpatialStructures::Graph g;

		int N1 = 1;
		int N2 = 2;

		g.addEdge(N1, N2, 100);
		g.Compress();
		ASSERT_EQ(g.size(), 2);
	}

	TEST(_Graph, GroupInsert) {
		HF::SpatialStructures::Node N1(1, 1, 2);
		HF::SpatialStructures::Node N2(2, 3, 4, 5);
		HF::SpatialStructures::Node N3(11, 22, 140);

		//g.addEdge(N1, N2, 5);
		//g.addEdge(N2, N1, 6);
		//g.addEdge(N1, N3, 10);

		std::vector<std::vector<int>> edges = {
			{1, 2},
			{2},
			{1}
		};
		std::vector<std::vector<float>> costs = {
			{1.0f, 2.5f},
			{54.0f},
			{39.0f}
		};

		std::vector<Node> nodes = { N1,N2,N3 };
		Graph g(edges, costs, nodes);

		assert(g.HasEdge(0, 1));
		assert(g.HasEdge(0, 2));
		assert(g.HasEdge(1, 2));
		assert(!g.HasEdge(2, 0));

		assert(g.hasKey(N1));
		assert(g.hasKey(N2));
		assert(g.hasKey(N3));
	}

	/* This is an example of a test case that the current graph can't handle
	TEST(_Graph, IntAndNode) {
		Graph g;
		HF::SpatialStructures::Node N1(1, 1, 2);
		HF::SpatialStructures::Node N2(2, 3, 4, 5);
		HF::SpatialStructures::Node N3(11, 22, 140);

		g.addEdge(N1, N2, 33);
		g.addEdge(4, 3, 33);

		g.NodeFromID(4);

	}
	*/

	bool Contains(const std::vector<int>& domain, int target) {
		for (int suspect : domain) {
			if (suspect == target)
				return true;
		}
		return false;
	}


	TEST(_Graph, GetEdges) {
		HF::SpatialStructures::Node N1(1, 1, 2);
		HF::SpatialStructures::Node N2(2, 3, 4);
		HF::SpatialStructures::Node N3(11, 22, 140);

		//g.addEdge(N1, N2, 5);
		//g.addEdge(N2, N1, 6);
		//g.addEdge(N1, N3, 10);

		std::vector<std::vector<int>> edges = {
			{1, 2},
			{2},
			{1}
		};
		std::vector<std::vector<float>> costs = {
			{1.0f, 2.5f},
			{54.0f},
			{39.0f}
		};

		std::vector<Node> nodes = { N1,N2,N3 };
		Graph g(edges, costs, nodes);

		std::vector<EdgeSet> edgesets = g.GetEdges();
		ASSERT_TRUE(edgesets.size() == edges.size());
		for (const EdgeSet edgeset : edgesets) {
			ASSERT_TRUE(edgeset.children.size() == edges[edgeset.parent].size());
			for (const IntEdge & edge : edgeset.children) {
				ASSERT_TRUE(Contains(edges[edgeset.parent], edge.child));
			}
		}

	}
}

// Assert that adding a new edge
// 1) Doesn't interfere with existing edges
// 2) Properly stores its own cost
TEST(_Graph, AddEdgeToNewCost) {

	// Create two nodes
	HF::SpatialStructures::Node N1(1, 1, 2);
	HF::SpatialStructures::Node N2(2, 3, 4);

	// Create a graph, add edges, then compress
	Graph g;
	g.Compress();
	g.addEdge(N1, N2, 0.39f);
	g.addEdge(N1, N2, 0.54f, "TestCost");
	
	// Get both edge sets
	auto default_edges =  g.GetEdges();
	auto testcost_edges =  g.GetEdges("TestCost");

	// Assert that the edges we defined exist in both seperate arrays.
	ASSERT_EQ(default_edges[0].children.size(), 1);
	ASSERT_EQ(default_edges[0].children[0].child, 1);
	ASSERT_EQ(default_edges[0].children[0].weight, 0.39f);

	ASSERT_EQ(testcost_edges[0].children.size(), 1);
	ASSERT_EQ(testcost_edges[0].children[0].child, 1);
	ASSERT_EQ(testcost_edges[0].children[0].weight, 0.54f);
}

// Assert that the above test holds for adding multiple edges.
TEST(_Graph, MultipleNewCostDoesntAffectDefault) {
	
	vector<IntEdge> StandardEdges = {
		{0,0.10f}, {1,0.11f}, {2,0.12f}
	};
	EdgeSet StandSet(3, StandardEdges);
	vector<IntEdge> AltCostEdges = {
		{0,0.20f}, {1,0.21f}, {2,0.22f}
	};
	EdgeSet AltSet(3, AltCostEdges);
	
	Graph g;
	g.Compress();
	g.AddEdges(StandSet);
	g.AddEdges(AltSet, "TestCost");

	// Get both edge sets
	auto default_edges =  g.GetEdges();
	auto testcost_edges =  g.GetEdges("TestCost");

	// Assert that the edges we defined exist in both seperate arrays.
	ASSERT_EQ(default_edges[3].children.size(), 3);
	ASSERT_EQ(default_edges[3].children[0].child, 0);
	ASSERT_EQ(default_edges[3].children[0].weight, 0.1f);

	ASSERT_EQ(testcost_edges[3].children.size(), 3);
	ASSERT_EQ(testcost_edges[3].children[0].child, 0);
	ASSERT_EQ(testcost_edges[3].children[0].weight, 0.2f);
}

TEST(_Graph, GetSubGraphMulti) {

	vector<IntEdge> StandardEdges = {
		{0,0.10f}, {1,0.11f}, {2,0.12f}
	};
	EdgeSet StandSet(3, StandardEdges);
	vector<IntEdge> AltCostEdges = {
		{0,0.20f}, {1,0.21f}, {2,0.22f}
	};
	EdgeSet AltSet(3, AltCostEdges);

	Graph g;
	g.Compress();
	g.AddEdges(StandSet);
	g.AddEdges(AltSet, "TestCost");

	// Get both edge sets
	auto default_edges = g.GetEdges();
	auto testcost_edges = g.GetEdges("TestCost");

	auto sg = g.GetSubgraph(3, "TestCost");

	ASSERT_EQ(sg.m_edges.size(), 3);
	ASSERT_EQ(sg.m_edges[1].score, 0.21f);
	ASSERT_EQ(sg.m_edges[2].score, 0.22f);


}

inline void CompareVectorsOfEdgeSets(const vector<EdgeSet> & E1, const vector<EdgeSet> & E2){
	ASSERT_EQ(E1.size(), E2.size());

	for (int i = 0; i < E1.size(); i++)
		ASSERT_EQ(E1[i], E2[i]);
}

TEST(_Graph, AddMultipleEdgeSetsToNewCost) {
	Graph g;
	
	// Add filler edges to the graph as a base set of edges
	g.Compress();
	vector<EdgeSet> filler_edges{
		{0, { {0, 9999.0f}, {1, 9999.0f}, {2, 9999.0f} }},
		{1, { {0, 9999.0f}, {1, 9999.0f}, {2, 9999.0f} }},
		{2, { {0, 9999.0f}, {1, 9999.0f }, {2, 9999.0f} }}
	};
	g.AddEdges(filler_edges);

	// Add actual edges we want to test with
	vector<EdgeSet> Edges{
		{0, { {0,0.00f}, {1,0.01f},	{2,0.02f} }},
		{1, { {0,0.10f}, {1,0.11f},	{2,0.12f} }},
		{2, { {0,0.20f}, { 1,0.21f }, { 2,0.22f} }}
	};
	g.AddEdges(Edges, "AltCost");

	// Compare the result of the graph's output with our own edges
	CompareVectorsOfEdgeSets(Edges, g.GetEdges("AltCost"));
}

TEST(_Graph, DefaultNameChange) {
	const std::string default_name = "DefaultTestName";

	Graph g(default_name);
	g.Compress();
	g.addEdge(0, 1, 100, default_name);
	ASSERT_TRUE(g.HasEdge(0, 1));

	// If this throws here, that means we're not adding
	// to the default cost type
	try {
		g.addEdge(0, 2, 100, "Non-Default-Name");
	}
	catch (std::out_of_range) {
		GTEST_SUCCEED();
	}
	catch (...){
		GTEST_FAIL("Other exception occured.");
	}
}

TEST(_Graph, ClearMulti) {
	const std::string alternate_name = "DefaultTestName";

	Graph g;
	g.Compress();
	g.addEdge(0, 1, 100);

	g.addEdge(0, 1, 150, alternate_name);
	ASSERT_TRUE(g.HasEdge(0, 1, false, alternate_name));
	
	g.Clear();
	ASSERT_FALSE(g.HasEdge(0, 1, false, alternate_name));

	g.addEdge(0, 1, 150);
	ASSERT_FALSE(g.HasEdge(0, 1, false, alternate_name));

}

TEST(_Graph, AlternateCSR) {
	
	// Add filler edges to the graph as a base set of edges
	Graph g;
	g.Compress();
	vector<EdgeSet> filler_edges{
		{0, { {0, 9999.0f}, {1, 9999.0f}, {2, 9999.0f} }},
		{1, { {0, 9999.0f}, {1, 9999.0f}, {2, 9999.0f} }},
		{2, { {0, 9999.0f}, {1, 9999.0f }, {2, 9999.0f} }}
	};
	g.AddEdges(filler_edges);

	// Add actual edges we want to test with
	vector<EdgeSet> Edges{
		{0, { {0,0.00f}, {1,0.01f},	{2,0.02f} }},
		{1, { {0,0.10f}, {1,0.11f},	{2,0.12f} }},
		{2, { {0,0.20f}, { 1,0.21f }, { 2,0.22f} }}
	};
	g.AddEdges(Edges, "AltCost");

	// Ensure they're different, and the sum equates to what we would expect
	CSRPtrs stand_csrptrs = g.GetCSRPointers();
	auto alt_csrptrs = g.GetCSRPointers("AltCost");
	const int num_nnz = stand_csrptrs.nnz;

	vector<float> stand_values(stand_csrptrs.data, stand_csrptrs.data + num_nnz);
	vector<float>  alt_values(alt_csrptrs.data, alt_csrptrs.data + num_nnz);

	ASSERT_FALSE(std::equal(stand_values.begin(), stand_values.end(), alt_values.begin()));
}

const string test_attribute = "test_attr";
const vector<Node> test_param_nodes = {
		{1,1,1}, {2,2,2}, {3,3,3},{4,4,4}, {5,5,5}
};

std::vector<int> GetIds(const Graph & G, const std::vector<Node> & nodes) {
	std::vector<int> ids(nodes.size(), -1);
	for (int i = 0; i < nodes.size(); i++)
		ids[i] = G.getID(nodes[i]);

	return ids;
}

Graph CreateNodeAttributeGraph() {

	//! [EX_AttrsToStrings]

	// Define nodes
	const vector<Node> Nodes = {
		{1,1,1}, {2,2,2}, {3,3,3},{4,4,4}, {5,5,5}
	};

	// Define the graph, compress it then add edges
	Graph G;
	G.addEdge(Nodes[0], Nodes[2], 2);
	G.addEdge(Nodes[0], Nodes[1], 1);
	G.addEdge(Nodes[3], Nodes[0], 3);
	G.addEdge(Nodes[2], Nodes[1], 4);
	G.addEdge(Nodes[0], Nodes[4], 555);
	G.Compress();

	// Get the ids of every node since the graph assigns them
	std::vector<int> ids(Nodes.size(), -1);
	for (int i = 0; i < Nodes.size(); i++)
		ids[i] = G.getID(Nodes[i]);

	// Create node attributes
	G.AddNodeAttribute(ids[0], test_attribute, "000");
	G.AddNodeAttribute(ids[1], test_attribute, "111");
	G.AddNodeAttribute(ids[2], test_attribute, "222");
	G.AddNodeAttribute(ids[3], test_attribute, "333");
	
	//! [EX_AttrsToStrings]
	return G;
}

inline float StringToFloat(const std::string& str_to_convert) {

	try {
		return std::stod(str_to_convert);
	}
	catch (std::invalid_argument) {
		return -1;
	}
}

inline std::vector<float> ConvertStringsToFloat(const std::vector<std::string>& strings) {
	std::vector<float> out_floats(strings.size());

	for (int i = 0; i < out_floats.size(); i++) 
		out_floats[i] = StringToFloat(strings[i]);
	return out_floats;
}


TEST(_Graph, AttrToParams_INCOMING) {
	Graph G = CreateNodeAttributeGraph();
	const auto ids = GetIds(G, test_param_nodes);
	const std::vector<std::string> attributes = G.GetNodeAttributes(test_attribute);

	//![EX_AttrsToStrings2]

	// Convert node attributes to graph costs based on the cost of the child node
	G.AttrToCost(test_attribute, "output_str", Direction::INCOMING);

	// Print out the cost of edge 3 to 0
	printf("0->1: %f\n", G.GetCost(ids[0], ids[1], "output_str"));

	//![EX_AttrsToStrings2]
	
	// Assert Correctness
	const std::vector<float> scores = ConvertStringsToFloat(attributes);
	ASSERT_EQ(scores[ids[0]], G.GetCost(ids[3], ids[0], "output_str"));
	ASSERT_EQ(scores[ids[1]], G.GetCost(ids[0], ids[1], "output_str"));
	ASSERT_EQ(scores[ids[0]], G.GetCost(ids[3], ids[0], "output_str"));
	ASSERT_EQ(scores[ids[1]], G.GetCost(ids[2], ids[1], "output_str"));
}

TEST(_Graph, AttrToParams_OUTGOING) {
	Graph G = CreateNodeAttributeGraph();
	const auto ids = GetIds(G, test_param_nodes);
	const std::vector<std::string> attributes = G.GetNodeAttributes(test_attribute);
	const std::vector<float> scores = ConvertStringsToFloat(attributes);

	// Convert node attributes to strings
	G.AttrToCost(test_attribute, "output_str", Direction::OUTGOING);

	// Assert Correctness
	ASSERT_EQ(scores[ids[3]], G.GetCost(ids[3], ids[0], "output_str"));
	ASSERT_EQ(scores[ids[0]], G.GetCost(ids[0], ids[1], "output_str"));
	ASSERT_EQ(scores[ids[3]], G.GetCost(ids[3], ids[0], "output_str"));
	ASSERT_EQ(scores[ids[2]], G.GetCost(ids[2], ids[1], "output_str"));
}

TEST(_Graph, AttrToParams_BOTH) {
	Graph G = CreateNodeAttributeGraph();
	const auto ids = GetIds(G, test_param_nodes);
	const std::vector<std::string> attributes = G.GetNodeAttributes(test_attribute);
	const std::vector<float> scores = ConvertStringsToFloat(attributes);

	// Convert node attributes to strings
	G.AttrToCost(test_attribute, "output_str", Direction::BOTH);

	// Assert Correctness
	ASSERT_EQ(scores[ids[3]] + scores[ids[0]], G.GetCost(ids[3], ids[0], "output_str"));
	ASSERT_EQ(scores[ids[0]] + scores[ids[1]], G.GetCost(ids[0], ids[1], "output_str"));
	ASSERT_EQ(scores[ids[3]] + scores[ids[0]], G.GetCost(ids[3], ids[0], "output_str"));
	ASSERT_EQ(scores[ids[2]] + scores[ids[1]], G.GetCost(ids[2], ids[1], "output_str"));
}

TEST(C_Graph, AttrToParams) {
	
	Graph G = CreateNodeAttributeGraph();
	const auto ids = GetIds(G, test_param_nodes);
	const std::vector<std::string> attributes = G.GetNodeAttributes(test_attribute);
	const std::vector<float> scores = ConvertStringsToFloat(attributes);

	
	// Convert node attributes to strings
	GraphAttrsToCosts(&G, test_attribute.c_str(), "output_str", Direction::BOTH);

	// Assert Correctness
	ASSERT_EQ(scores[ids[3]] + scores[ids[0]], G.GetCost(ids[3], ids[0], "output_str"));
	ASSERT_EQ(scores[ids[0]] + scores[ids[1]], G.GetCost(ids[0], ids[1], "output_str"));
	ASSERT_EQ(scores[ids[3]] + scores[ids[0]], G.GetCost(ids[3], ids[0], "output_str"));
	ASSERT_EQ(scores[ids[2]] + scores[ids[1]], G.GetCost(ids[2], ids[1], "output_str"));
}

TEST(_Rounding, addition_error)
{
	// define values as floats
	float start = 6.78400040f;
	float adder = 70.0f;

	// add floats and store as float
	float combined = start + adder;

	// define same as doubles
	double start_d = 6.78400040;
	double adder_d = 70.0;

	// add doubles and cast to float
	float combined_double = (float)(start_d + adder_d);
}
/*
TEST(_Rounding, Equality) {

	float f1 = 3.28399992f;
	float f2 = 3.28390002f;

	float rounded_f1 = roundhf(f1);
	float rounded_f2 = roundhf(f2);
	ASSERT_EQ(rounded_f1, rounded_f2);
	ASSERT_NE(std::hash<float>()(f1), std::hash<float>()(f2));
	ASSERT_EQ(std::hash<float>()(rounded_f1), std::hash<float>()(rounded_f2));
	ASSERT_EQ(std::hash<float>()(roundhf(f1)), std::hash<float>()(roundhf(f2)));
}

TEST(_Rounding, approach) {
	float f1 = 3.28399992f;
	float f2 = 3.28390002f;


	ASSERT_EQ(std::trunc(f1 / ROUNDING_PRECISION), std::trunc(f2/ROUNDING_PRECISION));
}

TEST(_Rounding, StepByStep) {
	float a1 = 3.28399991f;
	float a2 = 3.28394318f;

	float b1 = a1 / ROUNDING_PRECISION;
	float b2 = a2 / ROUNDING_PRECISION;

	float c1 = std::floor(b1);
	float c2 = std::floor(b2);

	float d1 = c1 * ROUNDING_PRECISION;
	float d2 = c2 * ROUNDING_PRECISION;

	ASSERT_EQ(d1, d2);
}


TEST(_Rounding, Seperate){
	float a1 = 3.28399991f;
	float a2 = 3.28394318f;

	a1 = a1 / ROUNDING_PRECISION;
	a1 = std::floor(a1);
	a1 = a1 * ROUNDING_PRECISION;

	a2 = a2 / ROUNDING_PRECISION;
	a2 = std::floor(a2);
	a2 = a2 * ROUNDING_PRECISION;

	ASSERT_EQ(a1, a2);
}


TEST(_Rounding, Combined){
	float a1 = 3.28399991f;
	float a2 = 3.28394318f;

	float r1 = std::floor(a1 / ROUNDING_PRECISION) * ROUNDING_PRECISION;
	float r2 = std::floor(a2 / ROUNDING_PRECISION) * ROUNDING_PRECISION;

	ASSERT_EQ(r1, r2);
}

/*!
TEST(_Node, Hashing) {
	Node N1(-2.42799997, -12.8568001, 3.28399992);
	Node N2(-2.42799997, -12.8568001, 3.28390002);

	ASSERT_EQ(N1, N2);
	ASSERT_EQ(roundhf(N1[2]), roundhf(N2[2]));
	ASSERT_EQ(std::hash<Node>()(N1), std::hash<Node>()(N2));
}
*/


namespace NodeTests {
    TEST(_Node, Distance) {
        Node N1(0, 0, 1);
        Node N2(0, 0, 0);

        ASSERT_EQ(N1.distanceTo(N2), 1);
    }
    TEST(_Node, Equality) {
        Node N1(0, 0, 1);
        Node N2(0, 0, 0);

        EXPECT_FALSE(N1 == N2);
        EXPECT_TRUE(N1 == N1);
        EXPECT_TRUE(N2 == N2);
    }

    TEST(_Node, RoundingAndEquality) {
		ASSERT_EQ(
			Node(-22.4279995,-12.856001, 1.254864040),
			Node( -22.4279995,-12.856001, 1.254854040)
		);
    }
}

///
///	The following are tests for the code samples for HF::SpatialStructures::Node
///

namespace NodeExampleTests {
	TEST(_Node, DefaultConstructor) {
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2, 456);

		/*
		std::cout << 12.0 << " " << node_0.x << std::endl;
		std::cout << 23.1 << " " << node_0.y << std::endl;
		std::cout << 34.2 << " " << node_0.z << std::endl;
		std::cout << 456 << " " << node_0.id << std::endl;
		*/
	}

	TEST(_Node, ParamConstructorNoID) {
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2);
	}

	TEST(_Node, ParamConstructorArray) {
		std::array<float, 3> pos = { 12.0, 23.1, 34.2 };
		HF::SpatialStructures::Node node(pos);
	}

	TEST(_node, ParamConstructorArrayTypeID) {
		std::array<float, 3> pos = { 12.0, 23.1, 34.2 };
		HF::SpatialStructures::Node node(pos, HF::SpatialStructures::NODE_TYPE::GRAPH, 456);
	}

	TEST(_node, DistanceTo) {
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2, 456);
		HF::SpatialStructures::Node node_1(45.3, 56.4, 67.5, 789);

		float euclidean_distance = node_0.distanceTo(node_1);
	}

	TEST(_node, AngleTo) {
		/// TODO code sample: line 40 of node.cpp - 'This needs an actual angle formula'
	}

	TEST(_node, DirectionTo) {
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2, 456);
		HF::SpatialStructures::Node node_1(45.3, 56.4, 67.5, 789);

		std::array<float, 3> direction_vector = node_0.directionTo(node_1);
	}

	TEST(_node, GetArray) {
		HF::SpatialStructures::Node node(12.0, 23.1, 34.2, 456);

		// An array is created from within the getArray() member function
		std::array<float, 3> arr = node.getArray();

		// ref_arr and ref_node have the same value, but
		// refer to different locations in memory --
		// arr does not consist of the same memory locations as
		// that of the coordinate fields within node.
		float& ref_arr = arr[0];
		float& ref_node = node.x;

		/*
		std::cout << "ref_arr: " << ref_arr << "\tref_node: " << ref_node << std::endl;
		std::cout << "ref_arr: " << &ref_arr << "\tref_node: " << &ref_node << std::endl;
		*/
	}

	TEST(_node, OperatorIndexByRef) {
		HF::SpatialStructures::Node node(12.0, 23.1, 34.2, 456);	// (x, y, z), ID

		float& position = node[1];			// access by reference
		position = 93.5;					// node.y is now 93.5

		/*
		std::cout << "node.y = " << node.y << std::endl;
		std::cout << "position = " << position << std::endl;
		*/
	}

	TEST(_node, OperatorIndexByVal) {
		HF::SpatialStructures::Node node(12.0, 23.1, 34.2, 456);	// (x, y, z), ID

		float position = node[1];			// access by reference
		position = 93.5;					// node.y is still 23.1

		/*
		std::cout << "node.y = " << node.y << std::endl;
		std::cout << "position = " << position << std::endl;
		*/
	}

	TEST(_node, OperatorEquality) {
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2, 456);
		HF::SpatialStructures::Node node_1(45.3, 56.4, 67.5, 789);

		bool same_position = node_0 == node_1;

		/*
		if (same_position) {
			std::cout << "Occupies the same space" << std::endl;
		} else {
			std::cout << "Different positions" << std::endl;
		}
		*/

		// same_position evaluates to false
	}

	TEST(_node, OperatorAssignment) {
		/* Cannot test -- not defined in node.cpp
		HF::SpatialStructures::Node node(12.0, 23.1, 34.2);
		std::cout << node.x << ", " << node.y << ", " << node.z << std::endl;
		std::array<float, 3> position = { 45.3, 56.4, 67.5 };

		node = position;

		std::cout << node.x << ", " << node.y << ", " << node.z << std::endl;
		*/
	}

	TEST(_node, OperatorNEquality) {
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2, 456);
		HF::SpatialStructures::Node node_1(45.3, 56.4, 67.5, 789);

		// Does the inverse of operator==.
		bool different_positions = node_0 != node_1;

		/*/
		if (different_positions) {
			std::cout << "Different positions" << std::endl;
		}
		else {
			std::cout << "Occupies the same space" << std::endl;
		}
		*/

		// different_positions evaluates to true
	}

	TEST(_node, OperatorMinus) {
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2, 456);
		HF::SpatialStructures::Node node_1(45.3, 56.4, 67.5, 789);

		// removed inline keyword from line 98 in node.cpp
		HF::SpatialStructures::Node node_2 = node_1 - node_0;

		// node_2 has values (x = 33.3, y = 33.3, z = 33.3, id = -1, type = NODE_TYPE::GRAPH)
		// id and type are given default values as per Node::Node(const std::array<float, 3>& position)

		//std::cout << node_2.x << " " << node_2.y << " " << node_2.z << std::endl;
	}

	TEST(_node, OperatorPlus) {
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2, 456);
		HF::SpatialStructures::Node node_1(45.3, 56.4, 67.5, 789);

		HF::SpatialStructures::Node node_2 = node_1 + node_0;

		// node_2 has values (x = 57.3, y = 79.5, z = 101.7, id = -1, type = NODE_TYPE::GRAPH)
		// id and type are given default values as per Node::Node(const std::array<float, 3>& position)

		//std::cout << node_2.x << " " << node_2.y << " " << node_2.z << std::endl;
	}

	TEST(_node, OperatorMultiply) {
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2, 456);
		HF::SpatialStructures::Node node_1(45.3, 56.4, 67.5, 789);

		HF::SpatialStructures::Node node_2 = node_1 * node_0;

		// node_2 has values (x = 543.6, y = 1302.84, z = 2308.5, id = -1, type = NODE_TYPE::GRAPH)
		// id and type are given default values as per Node::Node(const std::array<float, 3>& position)

		//std::cout << node_2.x << " " << node_2.y << " " << node_2.z << std::endl;
	}

	TEST(_node, OperatorLessThanConst) {
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2, 456);
		HF::SpatialStructures::Node node_1(45.3, 56.4, 67.5, 789);

		 // operator< compares ID fields of node_0 and node_1
		bool compare = node_0 < node_1;		// evaluates to true, since 456 < 789

		/*
		auto str = compare ? "true" : "false";
		std::cout << str << std::endl;
		*/
	}

	TEST(_node, OperatorLessThan) {
		// For this example, we are not concerned about the node coordinates.
		HF::SpatialStructures::Node node_0(0.0, 0.0, 0.0, 3);
		HF::SpatialStructures::Node node_1(0.0, 0.0, 0.0, 1);
		HF::SpatialStructures::Node node_2(0.0, 0.0, 0.0, 2);
		HF::SpatialStructures::Node node_3(0.0, 0.0, 0.0, 0);

		std::vector<Node> vec{ node_0, node_1, node_2, node_3 };

		// operator< sorts Node by ID, in non-decreasing order
		std::sort(vec.begin(), vec.end());	// uses natural ordering through operator<, non-const

		std::vector<Node>::iterator it = vec.begin();

		/*
		while (it != vec.end()) {
			std::cout << "Node ID: " << it->id << std::endl;
			++it;
		} // Node ID will print in order by ID, from smallest to largest
		*/
	}

	TEST(_node, OperatorGreaterThanConst) {
		HF::SpatialStructures::Node node_0(12.0, 23.1, 34.2, 456);
		HF::SpatialStructures::Node node_1(45.3, 56.4, 67.5, 789);

		// operator< compares ID fields of node_0 and node_1
		bool compare = node_0 > node_1;	// evaluates to false, since 456 < 789

		/*
		auto str = compare ? "true" : "false";
		std::cout << str << std::endl;
		*/
	}
}

///
///	The following are tests for the code samples for HF::SpatialStructures::Edge
///

namespace EdgeExampleTests {
	TEST(_edge, EdgeConstructor) {
		HF::SpatialStructures::Node node(12.0, 23.1, 34.2, 456);
		float score = 4.3f;

		HF::SpatialStructures::Edge edge(node, score, STEP::NOT_CONNECTED);
	}
}

///
///	The following are tests for the code samples for HF::SpatialStructures::Path
///

#include "path.h"
namespace PathExampleTests {
	TEST(_pathMember, OperatorEquality) {
		HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };
		HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
		HF::SpatialStructures::PathMember p2 = { 2.78f, 2 };

		bool is_true = p1 == p2;
		bool is_false = p0 == p1;

		// cost and node fields of p1 and p2 are identical, so equiv evaluates true
		// not_equiv evaluates false, since fields of p0 and p1 do not have matching values

		/*
		auto str_equiv = equiv ? "true" : "false";
		auto str_nequiv = not_equiv ? "true" : "false";
		std::cout << str_equiv << " " << str_nequiv << std::endl;
		*/
	}

	TEST(_pathMember, OperatorNEquality) {
		HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };
		HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
		HF::SpatialStructures::PathMember p2 = { 2.78f, 2 };

		bool is_true = p1 == p2;
		bool is_false = p0 != p1;

		// applies the inverse of operator== to evaluate operator!=
		// since fields of p1 and p2 have different values, is_true is in fact, true
		// is_false evaluates to true.

		/*
		auto str_equiv = is_true ? "true" : "false";
		auto str_nequiv = is_false ? "true" : "false";
		std::cout << str_equiv << " " << str_nequiv << std::endl;
		*/
	}

	TEST(_path, NoArgConstructor) {
		HF::SpatialStructures::Path path;
	}

	TEST(_path, ParamConstructor) {
		// Create the PathMembers
		HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };
		HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
		HF::SpatialStructures::PathMember p2 = { 1.64f, 1 };
		HF::SpatialStructures::PathMember p3 = { 9.35, 7 };

		// Create the container of PathMembers
		std::vector<HF::SpatialStructures::PathMember> members{p0, p1, p2, p3};

		// Create the path, using the container of PathMembers
		HF::SpatialStructures::Path path(members);
	}

	TEST(_path, AddNode) {
		// Create the PathMembers
		HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };
		HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
		HF::SpatialStructures::PathMember p2 = { 1.64f, 1 };
		HF::SpatialStructures::PathMember p3 = { 9.35, 7 };

		// Create the container of PathMembers
		std::vector<HF::SpatialStructures::PathMember> members{ p0, p1, p2, p3 };

		// Create the path, using the container of PathMembers
		HF::SpatialStructures::Path path(members);

		int node_id = 278;
		float cost = 8.92f;

		path.AddNode(node_id, cost);	// A PathMember is constructed within AddNode from node_id and cost
										// and is then appended to the underlying members vector (via push_back)
	}

	TEST(_path, Empty) {
		HF::SpatialStructures::Path mypath;

		// There are no PathMembers in path's members container.

		// if empty() returns true, that means the underlying members vector is of size 0 (no members)
		// otherwise, empty returns false. In this case, path.empty() returns true.
		std::string result = mypath.empty() ? "is empty" : "has at least one member";

		//std::cout << "The Path object " << result << std::endl;
	}

	TEST(_path, Size) {
		// Create the PathMembers
		HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };
		HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
		HF::SpatialStructures::PathMember p2 = { 1.64f, 1 };
		HF::SpatialStructures::PathMember p3 = { 9.35f, 7 };

		// Create the container of PathMembers
		std::vector<HF::SpatialStructures::PathMember> members{p0, p1, p2, p3};

		HF::SpatialStructures::Path path(members);			// Create the Path object, path
		path.AddNode(278, 3.14f);	// Add one more PathMember to path

		std::string result = path.size() >= 5 ? "at least 5 members" : "under 5 members";

		//std::cout << "The Path object has " << result << std::endl;
	}

	TEST(_path, Reverse) {
		// Create the PathMembers
		HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };
		HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
		HF::SpatialStructures::PathMember p2 = { 1.64f, 1 };
		HF::SpatialStructures::PathMember p3 = { 9.35f, 7 };

		// Create the container of PathMembers
		std::vector<HF::SpatialStructures::PathMember> members{p0, p1, p2, p3};

		HF::SpatialStructures::Path path(members);			// Create the Path object, path
		path.AddNode(278, 3.14f);	// Append one more PathMember to path

		path.Reverse();				// The order of the PathMembers within members is now that of
									// p3, p2, p1, p0

		//std::cout << path << std::endl;
	}

	TEST(_path, OperatorEquality) {
		HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };				// Create all the PathMember objects
		HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };

		HF::SpatialStructures::PathMember p2 = { 3.14f, 3 };
		HF::SpatialStructures::PathMember p3 = { 2.78f, 2 };

		HF::SpatialStructures::PathMember p4 = { 1.1f, 9 };
		HF::SpatialStructures::PathMember p5 = { 123.0f, 10 };

		std::vector<HF::SpatialStructures::PathMember> members_0{p0, p1};	// Create the HF::SpatialStructures::PathMember vectors
		std::vector<HF::SpatialStructures::PathMember> members_1{p2, p3};
		std::vector<HF::SpatialStructures::PathMember> members_2{p4, p5};
		std::vector<HF::SpatialStructures::PathMember> members_3{p1, p0};

		HF::SpatialStructures::Path path_0(members_0);						// Create the Path objects
		HF::SpatialStructures::Path path_1(members_1);
		HF::SpatialStructures::Path path_2(members_2);
		HF::SpatialStructures::Path path_3(members_3);

		bool same_values_same_order = path_0 == path_1;
		bool totally_different = path_0 == path_2;
		bool same_values_different_order = path_0 == path_3;

		// path_0 and path_1 share the same PathMember values, with an identical permutation,
		// so they are equivalent.

		//ASSERT_TRUE(same_values_different_order);

		// path_0 and path_2 are not equivalent, because they have completely different PathMember
		// values.

		//ASSERT_TRUE(!totally_different);

		// path_0 and path_3 are not equivalent, because although they have PathMember objects of
		// the same values, the order in which path_0 and path_3 have their member vectors arranged
		// are different.

		//ASSERT_TRUE(!same_values_different_order);
	}

	TEST(_path, OperatorIndex) {
		// Create the PathMembers
		HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };
		HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
		HF::SpatialStructures::PathMember p2 = { 1.64f, 1 };
		HF::SpatialStructures::PathMember p3 = { 9.35f, 7 };

		// Create the container of PathMembers
		std::vector<HF::SpatialStructures::PathMember> members{p0, p1, p2, p3};

		HF::SpatialStructures::Path path(members);							// Create the Path object, path

		const int desired_index = 2;
		HF::SpatialStructures::PathMember result = path[desired_index];	// a copy of the element at desired_index
														// within the internal members vector
														// is assigned to result

		//ASSERT_TRUE(&result != &path[desired_index]);
	}

	TEST(_path, GetPMPointer) {
		// Create the PathMembers
		HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };
		HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
		HF::SpatialStructures::PathMember p2 = { 1.64f, 1 };
		HF::SpatialStructures::PathMember p3 = { 9.35f, 7 };

		// Create the container of PathMembers
		std::vector<HF::SpatialStructures::PathMember> members{p0, p1, p2, p3};

		HF::SpatialStructures::Path path(members);							// Create the Path object, path

		HF::SpatialStructures::PathMember *ptr = path.GetPMPointer();

		// You now have a pointer to the underlying buffer of the members vector with in a PathMember.
		HF::SpatialStructures::PathMember *curr = ptr;
		HF::SpatialStructures::PathMember *finish = ptr + path.size();

		while (curr != finish) {
			std::cout << "Cost: " << curr->cost << " "
					  << "Node: " << curr->node << std::endl;
			++curr;
		}
	}

	TEST(_path, OperatorLessThanLessThan) {
		 // Create the PathMembers
		 HF::SpatialStructures::PathMember p0 = { 3.14f, 3 };
		 HF::SpatialStructures::PathMember p1 = { 2.78f, 2 };
		 HF::SpatialStructures::PathMember p2 = { 1.64f, 1 };
		 HF::SpatialStructures::PathMember p3 = { 9.35f, 7 };

			// Create the container of PathMembers
		 std::vector<HF::SpatialStructures::PathMember> members{p0, p1, p2, p3};

		 HF::SpatialStructures::Path path(members);							// Create the Path object, path

		 std::cout << path << std::endl;				// Output path to an ostream, like cout
		 // Output is:
			// (3) -3.14-> (2) -2.78-> (1) -1.64-> (7) -9.35->
	}
}

///
///	The following are tests for the code samples for HF::SpatialStructures::Graph
///

namespace GraphExampleTests {
	TEST(_csrptrs, AreValid) {

		 std::unique_ptr<float[]> data(new float[16]);
		 std::unique_ptr<int[]> outer_indices(new int[16]);
		 std::unique_ptr<int[]> inner_indices(new int[16]);

		 float* p_data = data.get();
		 int* p_outer_indices = outer_indices.get();
		 int* p_inner_indices = inner_indices.get();

		 HF::SpatialStructures::CSRPtrs csr = { 16, 16, 16, p_data, p_outer_indices, p_inner_indices };

		 bool validity = csr.AreValid();	// validity == true, since all pointer fields are non-null

		ASSERT_TRUE(validity);
	}

	TEST(_graph, ParamConstructor) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);

		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		HF::SpatialStructures::Graph graph(edges, distances, nodes);
	}

	TEST(_graph, NoArgConstructor) {
		HF::SpatialStructures::Graph graph;		// This represents an order-zero graph (null graph)
												// It lacks vertices and edges.
	}

	TEST(_graph, HasEdge_Array) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);

		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		HF::SpatialStructures::Graph graph(edges, distances, nodes);

		// Prepare {x, y, z} coordinates (positions)
		auto parent_pos = node_1.getArray();		// (2.0, 3.0, 4.0)
		auto child_pos = node_2.getArray();			// (11.0, 22.0, 140.0)

		// last argument can be true/false for undirected/directed graph respectively
		bool has_edge = graph.HasEdge(parent_pos, child_pos, true);

		ASSERT_TRUE(has_edge);
	}

	TEST(_graph, HasEdge_Node) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);

		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		HF::SpatialStructures::Graph graph(edges, distances, nodes);

		// last argument can be true/false for undirected/directed graph respectively
		bool has_edge = graph.HasEdge(node_1, node_2, true);

		ASSERT_TRUE(has_edge);
	}

	TEST(_graph, HasEdge_Int) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);

		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		HF::SpatialStructures::Graph graph(edges, distances, nodes);

		// last argument can be true/false for undirected/directed graph respectively
		bool has_edge = graph.HasEdge(0, 1, true);

		ASSERT_TRUE(has_edge);
	}

	TEST(_graph, Nodes) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);

		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		HF::SpatialStructures::Graph graph(edges, distances, nodes);

		// Nodes() returns a copy of the ordered_nodes field
		std::vector<HF::SpatialStructures::Node> nodes_from_graph = graph.Nodes();

		/*
		for (auto n : nodes_from_graph) {
			std::cout << "(" << n.x << ", " << n.y << ", " << n.z << ") : " << n.id << std::endl;
		}
		*/
	}

	TEST(_graph, GetUndirectedEdges) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);

		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		HF::SpatialStructures::Graph graph(edges, distances, nodes);

		// Retrieve the nodes from the graph, or use the original instance of
		// std::vector<HF::SpatialStructures::Node> passed to Graph upon instantiation
		std::vector<HF::SpatialStructures::Node> get_nodes = graph.Nodes();

		/*
		for (auto n : get_nodes) {
			std::cout << n.getArray() << std::endl;
		}
		*/

		std::cout << "\n";

		// nodes[index] yields an instance of Node that we can pass to GetUndirectedEdges.
		// Any node that exists with graph can be passed to this member function
		// to retrieve a vector of undirected edges.
		int index = 2;
		std::vector<HF::SpatialStructures::Edge> undirected_edges = graph.GetUndirectedEdges(get_nodes[index]);

		/*
		for (auto e : undirected_edges) {
			std::cout << e.child.getArray() << std::endl;
		}
		*/
	}

	TEST(_graph, GetEdges) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);

		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);

		// graph must be compressed, or a exception will be thrown

		// To brief, an EdgeSet has the following layout:
			//		struct EdgeSet {
			//			int parent;
			//			std::vector<IntEdge> children;
			//		};
			//
			//	An IntEdge has the following layout:
			//		struct IntEdge {
			//			int child;
			//			float weight;
			//		};

		// A std::vector<EdgeSet> is a Graph, in the form of IDs.
		std::vector<HF::SpatialStructures::EdgeSet> edge_set = graph.GetEdges();

		/*
		for (auto e : edge_set) {
			for (auto f : e.children) {
				std::cout << e.parent << "->" << "(" << f.child << ")" << std::endl;
			}
		}
		*/
	}

	TEST(_graph, AggregateGraph) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);

		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);

		// graph must be compressed, or a exception will be thrown
		HF::SpatialStructures::COST_AGGREGATE aggregate = HF::SpatialStructures::COST_AGGREGATE::AVERAGE;		// aggregate == 1 in this case

		// directed parameter may be true or false
		std::vector<float> aggregate_graph = graph.AggregateGraph(aggregate, true);

		/*
		for (auto a : aggregate_graph) {
			std::cout << a << std::endl;
		}
		*/
	}

	TEST(_graph, OperatorIndex) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);

		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);

		// Retrieve the nodes from the graph, or use the original instance of
		// std::vector<Node> passed to Graph upon instantiation
		std::vector<HF::SpatialStructures::Node> get_nodes = graph.Nodes();

		// nodes[index] yields an instance of Node that we can pass to GetUndirectedEdges.
		// Any node that exists with graph can be passed to this member function
		// to retrieve a vector of edges.
		int index = 2;
		HF::SpatialStructures::Node node = get_nodes[index];

		// Note that if node does not exist within graph, that an exception will be thrown.
		std::vector<HF::SpatialStructures::Edge> undirected_edges = graph[node];

		std::cout << node.getArray() << "->";
		for (auto e : undirected_edges) {
			std::cout << e.child.getArray() << std::endl;
		}
	}

	TEST(_graph, AddEdge_Node) {
		// be sure to #include "graph.h"

		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f);

		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);

		// Create a pair of nodes
		HF::SpatialStructures::Node n_parent(4.0f, 5.0f, 6.0f);
		HF::SpatialStructures::Node n_child(7.0f, 8.0f, 9.0f);

		graph.addEdge(n_parent, n_child);	// default score is 1.0f
	}

	TEST(_graph, AddEdge_Int) {
		// be sure to #include "graph.h"

		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);

		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);

		int parent = 1;
		int child = 2;

		graph.addEdge(parent, child, 1.0f);
		graph.Compress();
	}

	TEST(_graph, HasKey) {
		// be sure to #include "graph.h"

		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);

		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);

		HF::SpatialStructures::Node other_node(55.0f, 66.1f, 15.5f, 9510);	// Let's construct a Node we know is not in graph.
		bool has_key = graph.hasKey(other_node);	// other_node does not exist in graph, so has_key == false;

		// Likewise, if we pass a Node instance that indeed exists...

		// Retrieve the nodes from the graph, or use the original instance of
		// std::vector<Node> passed to Graph upon instantiation
		std::vector<HF::SpatialStructures::Node> get_nodes = graph.Nodes();

		// nodes[index] yields an instance of Node that we can pass to hasKey.
		// Any node that exists with graph can be passed to this member function
		// to determine if the graph has the node's key, or not.
		int index = 2;
		HF::SpatialStructures::Node good_node = get_nodes[index];

		has_key = graph.hasKey(good_node);		// now has_key is true

		ASSERT_TRUE(has_key);
	}

	TEST(_graph, NodesAsFloat3) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);

		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);

		// A container of std::array<float, 3> is constructed and populated within
		// NodesAsFloat3, and returned. Each array of 3 floats represents a Node's position
		// within the Cartesian coordinate system. { x, y, z }
		std::vector<std::array<float, 3>> nodes_as_floats = graph.NodesAsFloat3();

		// The two loops below will yield the same output
		for (auto n : graph.Nodes()) {
			std::cout << "(" << n.x << "," << n.y << "," << n.z << ")" << std::endl;
		}

		for (auto a : nodes_as_floats) {
			std::cout << a << std::endl;
		}
	}

	TEST(_graph, Size) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);

		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);

		int id_count = graph.size();		  // We retrieve the size of the node id count within graph

		ASSERT_TRUE(id_count == 3);
	}

	TEST(_graph, GetID) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);

		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);

		HF::SpatialStructures::Node other_node(55.0f, 66.1f, 15.5f, 9510);	// Let's construct a Node we know is not in graph.
		bool has_key = graph.hasKey(other_node);	// other_node does not exist in graph, so has_key == false;

		int ID = graph.getID(other_node);			// ID will assigned -1, because other_node is not a part of graph.

		// Likewise, if we pass a Node instance that indeed exists...

		// Retrieve the nodes from the graph, or use the original instance of
		// std::vector<Node> passed to Graph upon instantiation
		std::vector<HF::SpatialStructures::Node> get_nodes = graph.Nodes();

		// nodes[index] yields an instance of Node that we can pass to hasKey.
		// Any node that exists with graph can be passed to this member function
		// to determine if the graph has the node's key, or not.
		int index = 2;					// we assume for this example that index 2 is valid.
		HF::SpatialStructures::Node good_node = get_nodes[index];

		ID = graph.getID(good_node);	// ID > -1, i.e. it is a Node instance that exists within this Graph.

		std::cout << ID << std::endl;
	}

	TEST(_graph, Compress) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);

		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);

		// Create a pair of nodes
		HF::SpatialStructures::Node n_parent(4.0f, 5.0f, 6.0f);
		HF::SpatialStructures::Node n_child(7.0f, 8.0f, 9.0f);

		graph.addEdge(n_parent, n_child);	// default score is 1.0f

		// In order to use GetEdges, or AggregateGraph, we must compress our graph instance
		graph.Compress();						// GetEdges and AggregateGraph are now usable
	}

	TEST(_graph, GetCSRPointers) {
		// be sure to #include "graph.h"

		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);

		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);

		// Create a pair of nodes
		HF::SpatialStructures::Node n_parent(4.0f, 5.0f, 6.0f);
		HF::SpatialStructures::Node n_child(7.0f, 8.0f, 9.0f);

		graph.addEdge(n_parent, n_child);	// default score is 1.0f

		// Graph will be compressed automatically be GetCSRPointers
		CSRPtrs returned_csr = graph.GetCSRPointers();
	}

	TEST(_graph, NodeFromID) {
		// be sure to #include "graph.h"

		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);

		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);

		// Let's retrieve node_1.
		int desired_node_id = 2;
		HF::SpatialStructures::Node node_from_id = graph.NodeFromID(desired_node_id);

		// Note that NodeFromID ceases to work if the id argument provided does not exist as an ID among
		// the nodes within graph
		ASSERT_EQ(node_from_id.id, desired_node_id);
	}

	TEST(_graph, Clear) {
		// Create the nodes
		HF::SpatialStructures::Node node_0(1.0f, 1.0f, 2.0f, 4);
		HF::SpatialStructures::Node node_1(2.0f, 3.0f, 4.0f, 5);
		HF::SpatialStructures::Node node_2(11.0f, 22.0f, 140.0f, 6);

		// Create a container (vector) of nodes
		std::vector<HF::SpatialStructures::Node> nodes = { node_0, node_1, node_2 };

		// Create matrices for edges and distances, edges.size() == distances().size()
		std::vector<std::vector<int>> edges = { { 1, 2 }, { 2 }, { 1 } };
		std::vector<std::vector<float>> distances = { { 1.0f, 2.5f }, { 54.0f }, { 39.0f } };

		// Now you can create a Graph - note that nodes, edges, and distances are passed by reference
		// Note: graph is compressed upon instantiation
		HF::SpatialStructures::Graph graph(edges, distances, nodes);

		// If we want to remove all nodes and edges from graph, we may do so with Clear:
		graph.Clear();						// active_edge_matrix is zeroed out, buffer is squeezed,
											// triplets are also cleared, and needs_compression == true

		auto v = graph.NodesAsFloat3();
		for (auto n : v) {
			std::cout << n << std::endl;
		}
	}

	///
	///	The following are tests for the code samples for HF::SpatialStructures::Constants
	///

	namespace ConstantsExampleTests {
		TEST(_functions, RoundHF) {
			// std::round from the cmath library is used in roundhf.
			// Precision is to the nearest ten-thousandth
			const float my_pi = 3.141592653589793238462643383279502884197169399375105820974944592307816406286;
			float rounded = HF::SpatialStructures::roundhf(my_pi);	// 	rounded == 3.1416
			//std::cout << rounded << std::endl;
		}
	}
	
	// This just tests that attributes can be added without a runtime error. See GetNodeAttributes
	// For a test of correctness
	TEST(_graph, AddNodeAttribute) {
		Graph g;
		g.addEdge(0, 1, 1);	g.addEdge(0, 2, 1);	g.addEdge(1, 3, 1);	g.addEdge(1, 4, 1);
		g.addEdge(2, 4, 1);	g.addEdge(3, 5, 1);	g.addEdge(3, 6, 1);	g.addEdge(4, 5, 1);
		g.addEdge(5, 6, 1);	g.addEdge(5, 7, 1);	g.addEdge(5, 8, 1);	g.addEdge(4, 8, 1);
		g.addEdge(6, 7, 1);	g.addEdge(7, 8, 1);

		g.AddNodeAttribute(0, "cross slope", "5.1");
		g.AddNodeAttribute(1, "cross slope", "24.1");
		g.AddNodeAttribute(2, "default", "9");
		g.AddNodeAttribute(3, "cross slope", "7.1");

		auto attrs = g.GetNodeAttributes("cross slope");
		ASSERT_TRUE(attrs.size() == g.size());

		// Create a new float attribute that will eventually be turned into a string attribute
		g.AddNodeAttributeFloat(0, "float_to_string_attribute", 5.1);
		auto pre_string_add_float_attr = g.GetNodeAttributesFloat("float_to_string_attribute");
		ASSERT_TRUE(pre_string_add_float_attr.size() == g.size());

		// Add a string as a value to a float attribute, 
		// which should convert the entire attribute to a string attribute
		g.AddNodeAttribute(0, "float_to_string_attribute", "1.5");
		auto string_attrs = g.GetNodeAttributes("float_to_string_attribute");
		ASSERT_TRUE(string_attrs.size() == g.size());

		// Check that the attribute is no longer a float attribute
		auto post_string_add_float_attr = g.GetNodeAttributesFloat("float_to_string_attribute");
		ASSERT_TRUE(post_string_add_float_attr.size() == 0);
	}

	TEST(_graph, AddNodeAttributeFloat)
	{
		Graph g;
		g.addEdge(0, 1, 1);	g.addEdge(0, 2, 1);	g.addEdge(1, 3, 1);	g.addEdge(1, 4, 1);
		g.addEdge(2, 4, 1);	g.addEdge(3, 5, 1);	g.addEdge(3, 6, 1);	g.addEdge(4, 5, 1);
		g.addEdge(5, 6, 1);	g.addEdge(5, 7, 1);	g.addEdge(5, 8, 1);	g.addEdge(4, 8, 1);
		g.addEdge(6, 7, 1);	g.addEdge(7, 8, 1);

		const string& attr = "test attribute";
		// Add floats for test attribute
		g.AddNodeAttributeFloat(0, attr, 5.1);
		g.AddNodeAttributeFloat(1, attr, 24.1);
		g.AddNodeAttributeFloat(2, attr, 9);
		g.AddNodeAttributeFloat(3, attr, 7.1);

		// Check that test attribute is a float attribute with all nodes...
		auto float_attrs = g.GetNodeAttributesFloat(attr);
		ASSERT_TRUE(float_attrs.size() == g.size());

		// And not a string attribute
		auto string_attrs = g.GetNodeAttributes(attr);
		ASSERT_TRUE(string_attrs.size() == 0);

	}

	// This just tests that attributes can be added. See GetNodeAttributes
	// For a test of correctness
	TEST(_graph, AddNodeAttributes) {
		Graph g;
		g.addEdge(0, 1, 1);	g.addEdge(0, 2, 1);	g.addEdge(1, 3, 1);	g.addEdge(1, 4, 1);
		g.addEdge(2, 4, 1);	g.addEdge(3, 5, 1);	g.addEdge(3, 6, 1);	g.addEdge(4, 5, 1);
		g.addEdge(5, 6, 1);	g.addEdge(5, 7, 1);	g.addEdge(5, 8, 1);	g.addEdge(4, 8, 1);
		g.addEdge(6, 7, 1);	g.addEdge(7, 8, 1);

		std::vector<int> ids{ 1, 3, 5, 7 };
		std::string attr_type = "cross slope";
		std::vector<std::string> scores{ "1.4", "2.0", "2.8", "4.0" };
		
		g.AddNodeAttributes(ids, attr_type, scores);

		auto attrs = g.GetNodeAttributes(attr_type);
		ASSERT_TRUE(attrs.size() == g.size());
	}

	TEST(_graph, AddNodeAttributesFloat)
	{
		Graph g;
		g.addEdge(0, 1, 1);	g.addEdge(0, 2, 1);	g.addEdge(1, 3, 1);	g.addEdge(1, 4, 1);
		g.addEdge(2, 4, 1);	g.addEdge(3, 5, 1);	g.addEdge(3, 6, 1);	g.addEdge(4, 5, 1);
		g.addEdge(5, 6, 1);	g.addEdge(5, 7, 1);	g.addEdge(5, 8, 1);	g.addEdge(4, 8, 1);
		g.addEdge(6, 7, 1);	g.addEdge(7, 8, 1);

		std::vector<int> ids{ 1, 3, 5, 7 };
		std::string attr_type = "test attribute";
		std::vector<float> scores{ 1.4, 2.0, 2.8, 4.0 };

		g.AddNodeAttributesFloat(ids, attr_type, scores);

		// Check that all values were added to the float map
		auto float_attrs = g.GetNodeAttributesFloat(attr_type);
		ASSERT_TRUE(float_attrs.size() == g.size());

		// And not a string map
		auto string_attrs = g.GetNodeAttributes(attr_type);
		ASSERT_TRUE(string_attrs.size() == 0);


	}

	// If this fails then the values of the returned attributes don't match the input
	TEST(_graph, GetNodeAttributes) {

		// Create the graph, and add edges
		Graph g;
		g.addEdge(0, 1, 1); g.addEdge(0, 2, 1);	g.addEdge(1, 3, 1);	g.addEdge(1, 4, 1);
		g.addEdge(2, 4, 1);	g.addEdge(3, 5, 1);	g.addEdge(3, 6, 1);	g.addEdge(4, 5, 1);
		g.addEdge(5, 6, 1);	g.addEdge(5, 7, 1);	g.addEdge(5, 8, 1);	g.addEdge(4, 8, 1);
		g.addEdge(6, 7, 1);	g.addEdge(7, 8, 1);

		// Add node attributes
		g.AddNodeAttribute(0, "cross slope", "5.1");
		g.AddNodeAttribute(1, "cross slope", "24.1");
		g.AddNodeAttribute(2, "default", "9");
		g.AddNodeAttribute(3, "cross slope", "7.1");

		// Get the node attributes for cross slope
		auto attrs = g.GetNodeAttributes("cross slope");

		// The size of the output array should be equal to the size of the graph
		const int scores_out_size = g.size();
		ASSERT_EQ(attrs.size(), scores_out_size);

		// Compare the results to the expected scores
		vector<string> expected_scores = { "5.1", "24.1", "", "7.1", "", "", "", "", ""};
		for (int i = 0; i < scores_out_size; i++)
		{
			const auto& score = attrs[i];
			ASSERT_EQ(expected_scores[i], score);
			std::cout << "attribute: " << score << std::endl;
		}


	}

	TEST(_graph, GetNodeAttributesByID) {
		Graph g;
		g.addEdge(0, 1, 1); g.addEdge(0, 2, 1);	g.addEdge(1, 3, 1);	g.addEdge(1, 4, 1);
		g.addEdge(2, 4, 1);	g.addEdge(3, 5, 1);	g.addEdge(3, 6, 1);	g.addEdge(4, 5, 1);
		g.addEdge(5, 6, 1);	g.addEdge(5, 7, 1);	g.addEdge(5, 8, 1);	g.addEdge(4, 8, 1);
		g.addEdge(6, 7, 1);	g.addEdge(7, 8, 1);

		vector<int> ids = { 0, 3, 4, 8 };
		std::string testattribute = "testattribute";
		g.AddNodeAttribute(0, testattribute, "5.1");
		g.AddNodeAttribute(3, testattribute, "7.1");
		g.AddNodeAttribute(4, testattribute, "2.3");
		g.AddNodeAttribute(8, testattribute, "1.0");
		auto attrs = g.GetNodeAttributesByID(ids, testattribute);
		vector<string> expected_scores = { "5.1", "7.1", "2.3", "1.0" };
		int expected_scores_size = expected_scores.size();
		ASSERT_EQ(attrs.size(), expected_scores_size);
		for (int i = 0; i < expected_scores_size; i++)
		{
			ASSERT_EQ(attrs[i], expected_scores[i]);
		}
	}

	TEST(_graph, GetNodeAttributesFloat)
	{
		Graph g;
		g.addEdge(0, 1, 1); g.addEdge(0, 2, 1);	g.addEdge(1, 3, 1);	g.addEdge(1, 4, 1);
		g.addEdge(2, 4, 1);	g.addEdge(3, 5, 1);	g.addEdge(3, 6, 1);	g.addEdge(4, 5, 1);
		g.addEdge(5, 6, 1);	g.addEdge(5, 7, 1);	g.addEdge(5, 8, 1);	g.addEdge(4, 8, 1);
		g.addEdge(6, 7, 1);	g.addEdge(7, 8, 1);

		vector<int> ids = { 0, 3, 4, 8 };
		std::string testattribute = "testattribute";
		g.AddNodeAttributeFloat(0, testattribute, 5.1);
		g.AddNodeAttributeFloat(3, testattribute, 7.1);
		g.AddNodeAttributeFloat(4, testattribute, 2.3);
		g.AddNodeAttributeFloat(8, testattribute, 1.0);

		auto float_attrs = g.GetNodeAttributesFloat(testattribute);
		vector<float> expected_scores = { 5.1, 0.0, 0.0, 7.1, 2.3, 0, 0, 0, 1.0 };
		int expected_scores_size = expected_scores.size();
		ASSERT_EQ(float_attrs.size(), expected_scores_size);

		for (int i = 0; i < expected_scores_size; i++)
		{
			ASSERT_EQ(float_attrs[i], expected_scores[i]);
		}

	}

	TEST(_graph, GetNodeAttributesByIDFloat)
	{
		Graph g;
		g.addEdge(0, 1, 1); g.addEdge(0, 2, 1);	g.addEdge(1, 3, 1);	g.addEdge(1, 4, 1);
		g.addEdge(2, 4, 1);	g.addEdge(3, 5, 1);	g.addEdge(3, 6, 1);	g.addEdge(4, 5, 1);
		g.addEdge(5, 6, 1);	g.addEdge(5, 7, 1);	g.addEdge(5, 8, 1);	g.addEdge(4, 8, 1);
		g.addEdge(6, 7, 1);	g.addEdge(7, 8, 1);

		vector<int> ids = { 0, 3, 4, 8 };
		std::string testattribute = "testattribute";
		g.AddNodeAttributeFloat(0, testattribute, 5.1);
		g.AddNodeAttributeFloat(3, testattribute, 7.1);
		g.AddNodeAttributeFloat(4, testattribute, 2.3);
		g.AddNodeAttributeFloat(8, testattribute, 1.0);

		vector<int> subset_ids = { 0, 4 };
		auto float_attrs = g.GetNodeAttributesByIDFloat(subset_ids, testattribute);

		vector<float> expected_scores = { 5.1, 2.3 };
		int expected_scores_size = expected_scores.size();

		ASSERT_EQ(float_attrs.size(), expected_scores_size);

		for (int i = 0; i < expected_scores_size; i++)
		{
			ASSERT_EQ(float_attrs[i], expected_scores[i]);
		}
	}

	TEST(_graph, AttributeValueMapsCheck)
	{
		Graph g;
		g.addEdge(0, 1, 1); g.addEdge(0, 2, 1);	g.addEdge(1, 3, 1);	g.addEdge(1, 4, 1);
		g.addEdge(2, 4, 1);	g.addEdge(3, 5, 1);	g.addEdge(3, 6, 1);	g.addEdge(4, 5, 1);
		g.addEdge(5, 6, 1);	g.addEdge(5, 7, 1);	g.addEdge(5, 8, 1);	g.addEdge(4, 8, 1);
		g.addEdge(6, 7, 1);	g.addEdge(7, 8, 1);

		vector<int> ids = { 0, 1, 2, 3, 4, 5, 6, 7, 8};
		vector<std::string> float_test_attributes = { "float_test_attribute1", "float_test_attribute2", "float_test_attribute3", "float_test_attribute4", "float_test_attribute5"};
		vector<std::string> string_test_attributes = { "string_test_attribute1", "string_test_attribute2", "string_test_attribute3", "string_test_attribute4", "string_test_attribute5"};
		ASSERT_EQ(float_test_attributes.size(), string_test_attributes.size());
		vector<vector<float>> float_scores = { { 0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0 },
												{ 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0, 17.0 } };
		vector<vector<std::string>> string_scores = { { "0.0", "1.0", "2.0", "3.0", "4.0", "5.0", "6.0", "7.0", "8.0"},
													{ "9.0", "10.0", "11.0", "12.0", "13.0", "14.0", "15.0", "16.0", "17.0"} };
		for (int i = 0; i < float_test_attributes.size(); i++)
		{
			g.AddNodeAttributesFloat(ids, float_test_attributes[i], float_scores[i%2]);
			g.AddNodeAttributes(ids, string_test_attributes[i], string_scores[i%2]);
		}

		for (int i = 0; i < float_test_attributes.size(); i++)
		{
			vector<float> float_attrs = g.GetNodeAttributesFloat(float_test_attributes[i]);
			ASSERT_TRUE(float_attrs.size() == g.size());
			for (int j = 0; j < g.size(); j++)
			{
				ASSERT_EQ(float_attrs[j], float_scores[i%2][j]);
			}
		}

		for (int i = 0; i < string_test_attributes.size(); i++)
		{
			vector<std::string> string_attrs = g.GetNodeAttributes(string_test_attributes[i]);
			ASSERT_TRUE(string_attrs.size() == g.size());
			for (int j = 0; j < g.size(); j++)
			{
				ASSERT_EQ(string_attrs[j], string_scores[i%2][j]);
			}
		}

		for (int i = 0; i < float_test_attributes.size(); i++)
		{
			vector<float> string_name_float_attrs = g.GetNodeAttributesFloat(string_test_attributes[i]);
			vector<std::string> float_name_string_attrs = g.GetNodeAttributes(float_test_attributes[i]);
			ASSERT_TRUE(string_name_float_attrs.size() == 0);
			ASSERT_TRUE(float_name_string_attrs.size() == 0);
		}
	}
	// Assert that clearing a score from the graph returns an empty array next time
	// it's called, as the function should gaurantee.
	TEST(_graph, ClearNodeAttributes) {

		// Create a graph, add edges
		Graph g;
		g.addEdge(0, 1, 1);	g.addEdge(0, 2, 1);	g.addEdge(1, 3, 1);	g.addEdge(1, 4, 1);
		g.addEdge(2, 4, 1);	g.addEdge(3, 5, 1);	g.addEdge(3, 6, 1);	g.addEdge(4, 5, 1);
		g.addEdge(5, 6, 1);	g.addEdge(5, 7, 1);	g.addEdge(5, 8, 1);	g.addEdge(4, 8, 1);
		g.addEdge(6, 7, 1);	g.addEdge(7, 8, 1);

		// Add node attributes
		g.AddNodeAttribute(0, "cross slope", "5.1");
		g.AddNodeAttribute(1, "cross slope", "24.1");
		g.AddNodeAttribute(2, "default", "9");
		g.AddNodeAttribute(3, "cross slope", "7.1");

		// Clear the node attributes of cross_slope. 
		g.ClearNodeAttributes("cross slope");

		// Get the node attributes of cross slope. If it was successfully cleared,
		// then this should be an empty array.
		auto attrs = g.GetNodeAttributes("cross slope");
		ASSERT_TRUE(attrs.empty());
	}

	TEST(_graph, GetEdgesCostName) {
		///
		/// TODO test std::vector<EdgeSet> Graph::GetEdges(const string& cost_name);
		///

		///
		/// Requires methods from #24, Multiple Costs for the Graph
		/// in order to properly test this.
		///
		/// (i.e. Adding edges to a graph with a desired cost type)
		///
	}
}

	namespace ConstantsExampleTests {
		TEST(_functions, RoundHF) {
			// std::round from the cmath library is used in roundhf.
			// Precision is to the nearest ten-thousandth
			const float my_pi = 3.141592653589793238462643383279502884197169399375105820974944592307816406286;
			float rounded = HF::SpatialStructures::roundhf(my_pi);	// 	rounded == 3.1416
			//std::cout << rounded << std::endl;
		}
	}

namespace CInterfaceTests {

	// Verify that some attributes can be added. Doesn't verify any more than GetNodeAttributes.
	// Perhaps this should just be checking that it returns a success in the case that it's
	// preconditions are fulfilled.
	TEST(_graphCInterface, AddNodeAttributes) {
		// Create a graph
		Graph g;
		g.addEdge(0, 1, 1);	g.addEdge(0, 2, 1);	g.addEdge(1, 3, 1);	g.addEdge(1, 4, 1);
		g.addEdge(2, 4, 1);	g.addEdge(3, 5, 1);	g.addEdge(3, 6, 1);	g.addEdge(4, 5, 1);
		g.addEdge(5, 6, 1);	g.addEdge(5, 7, 1);	g.addEdge(5, 8, 1);	g.addEdge(4, 8, 1);
		g.addEdge(6, 7, 1);	g.addEdge(7, 8, 1);


		// Add some node attributes
		std::vector<int> ids{ 1, 3, 5, 7 };
		std::string attr_type = "cross slope";
		const char* scores[4] = { "1.4", "2.0", "2.8", "4.0" };
		AddNodeAttributes(&g, ids.data(), attr_type.c_str(), scores, ids.size());

		// Assert that atleast that many attributes were added
		ASSERT_TRUE(g.GetNodeAttributes(attr_type).size() == g.size());
		// Assert that no float attribute was created
		ASSERT_TRUE(g.GetNodeAttributesFloat(attr_type).size() == 0);
	}

	TEST(_graphCInterface, AddNodeAttributesFloat)
	{
		// Create a graph
		Graph g;
		g.addEdge(0, 1, 1);	g.addEdge(0, 2, 1);	g.addEdge(1, 3, 1);	g.addEdge(1, 4, 1);
		g.addEdge(2, 4, 1);	g.addEdge(3, 5, 1);	g.addEdge(3, 6, 1);	g.addEdge(4, 5, 1);
		g.addEdge(5, 6, 1);	g.addEdge(5, 7, 1);	g.addEdge(5, 8, 1);	g.addEdge(4, 8, 1);
		g.addEdge(6, 7, 1);	g.addEdge(7, 8, 1);

		// Add node attrbutes
		std::vector<int> ids{ 1, 3, 5, 7 };
		std::string attr_type = "testattribute";
		const float scores[4] = { 1.4, 2.0, 2.8, 4.0 };
		AddNodeAttributesFloat(&g, ids.data(), attr_type.c_str(), scores, ids.size());

		// Assert that atleast that many attributes were added
		ASSERT_TRUE(g.GetNodeAttributesFloat(attr_type).size() == g.size());
		// Assert that no string attribute was created
		ASSERT_TRUE(g.GetNodeAttributes(attr_type).size() == 0);
	}

	// Verify that the contents of GetNodeAttributes matches the input to AddNodeAttributes
	// For the C-Interface. Issues here can also indicate heap corruption and malformed strings
	TEST(_graphCInterface, GetNodeAttributes) {

		// Create a graph and add edges
		Graph g;
		g.addEdge(0, 1, 1); g.addEdge(0, 2, 1); g.addEdge(1, 3, 1); g.addEdge(1, 4, 1);
		g.addEdge(2, 4, 1); g.addEdge(3, 5, 1); g.addEdge(3, 6, 1); g.addEdge(4, 5, 1);
		g.addEdge(5, 6, 1); g.addEdge(5, 7, 1); g.addEdge(5, 8, 1); g.addEdge(4, 8, 1);
		g.addEdge(6, 7, 1);	g.addEdge(7, 8, 1);

		// Create a vector of node IDs and their corresponding values for our attribute
		vector<int> ids{ 1, 3, 5, 7 };
		string attr_type = "cross slope";
		const char* scores[4] = { "1.4", "2.0", "2.8", "4.0" };

		// Add node attributes to the graph
		AddNodeAttributes(&g, ids.data(), attr_type.c_str(), scores, ids.size());

		// Allocate an array of char arrays to meet the preconditions of GetNodeAttributes
		char** scores_out = new char* [g.size()];
		int scores_out_size = 0;

		// By the postconditions of GetNodeAttributes, this should update scores_out,
		// and scores_out_size with the variables we need
		GetNodeAttributes(&g, attr_type.c_str(), scores_out, &scores_out_size);

		// Assert that the size of the output array matches the number of nodes in the graph
		ASSERT_EQ(g.size(), scores_out_size);

		// Assert that we can get the scores from this array
		for (int i = 0; i < scores_out_size; i++)
		{
			// Convert score at this index to a string. 
			string score = scores_out[i];

			// If this fails, the char array doesn't have a null terminator, which caused
			// the constructor of std::string to run into adjacent memory until it found
			// a null terminator elsewhere. Several string functions rely on the
			// char array having a null terminator, so ensure it's being copied
			// over properly by GetNodeAttributes()!
			ASSERT_TRUE(score.length() == 3 || score.length() == 0); 
			
			// If it's in our input array, ensure that the score at this value
			// matches the one we passed
			auto itr = std::find(ids.begin(), ids.end(), i); 
			if (itr != ids.end())
			{
				// Get the index of the id in the scores array so we
				// can compare use it to get our input score at that
				// index as well.
				int index = std::distance(ids.begin(), itr);


				// Failures at either this or the assert below it could indicate
				// problems in AddNodeAttributes as well 
				ASSERT_EQ(scores[index], score);
			}
			else
				ASSERT_EQ("", score); 

		}

		// Deallocate the contents of scores_out by calling C_Interface function, then
		// deallocate scores_out since we're the ones who allocated that with new. 
		DeleteScoreArray(scores_out, scores_out_size);
		delete[] scores_out;

	}

	TEST(_graphCInterface, GetNodeAttributesFloat)
	{
		Graph g;
		g.addEdge(0, 1, 1); g.addEdge(0, 2, 1); g.addEdge(1, 3, 1); g.addEdge(1, 4, 1);
		g.addEdge(2, 4, 1); g.addEdge(3, 5, 1); g.addEdge(3, 6, 1); g.addEdge(4, 5, 1);
		g.addEdge(5, 6, 1); g.addEdge(5, 7, 1); g.addEdge(5, 8, 1); g.addEdge(4, 8, 1);
		g.addEdge(6, 7, 1);	g.addEdge(7, 8, 1);

		std::vector<int> ids{ 1, 3, 5, 7 };
		std::string attr_type = "testattribute";
		const float scores[4] = { 1.4, 2.0, 2.8, 4.0 };

		AddNodeAttributesFloat(&g, ids.data(), attr_type.c_str(), scores, ids.size());

		float* scores_out = new float[g.size()];
		int scores_out_size = 0;

		GetNodeAttributesFloat(&g, attr_type.c_str(), scores_out, &scores_out_size);
		ASSERT_EQ(scores_out_size, g.size());

		for (int i = 0; i < scores_out_size; i++)
		{
			// Convert score at this index to a float.
			float score = scores_out[i];

			// If it's in our input array, ensure that the score at this value
			// matches the one we passed
			auto itr = std::find(ids.begin(), ids.end(), i);
			if (itr != ids.end())
			{
				// Get the index of the id in the scores array so we
				// can compare use it to get our input score at that
				// index as well.
				int index = std::distance(ids.begin(), itr);


				// Failures at either this or the assert below it could indicate
				// problems in AddNodeAttributes as well 
				ASSERT_EQ(scores[index], score);
			}
			else
				ASSERT_EQ(0.0, score);

		}
		
		delete[] scores_out;
	}
	TEST(_graphCInterface, GetNodeAttributesByID) {
		// Create a graph and add edges
		Graph g;
		g.addEdge(0, 1, 1); g.addEdge(0, 2, 1); g.addEdge(1, 3, 1); g.addEdge(1, 4, 1);
		g.addEdge(2, 4, 1); g.addEdge(3, 5, 1); g.addEdge(3, 6, 1); g.addEdge(4, 5, 1);
		g.addEdge(5, 6, 1); g.addEdge(5, 7, 1); g.addEdge(5, 8, 1); g.addEdge(4, 8, 1);
		g.addEdge(6, 7, 1);	g.addEdge(7, 8, 1);

		// Create a vector of node IDs and their corresponding values for our attribute
		std::vector<int> ids = { 1, 2, 5, 7, 8 };
		std::string attr = "testattribute";
		const char* scores[5]  = {"1.0", "2.0", "3.0", "4.0", "5.0"};

		// Add node attributes to the graph
		AddNodeAttributes(&g, ids.data(), attr.c_str(), scores, ids.size());

		// What we expect to return from our call to GetNodeAttributesByID
		std::vector<int> subset_ids = { 1, 5, 7 };
		std::vector<std::string> expected_scores_out = { "1.0", "3.0", "4.0" };
		int expected_scores_out_size = 3;

		// Allocate an array of char arrays to meet the preconditions of GetNodeAttributesByID
		char** scores_out = new char* [subset_ids.size()];
		int scores_out_size = 0;
		
		// Get node attributes for the specified nodes
		GetNodeAttributesByID(&g, subset_ids.data(), attr.c_str(), subset_ids.size(), scores_out, &scores_out_size);

		// We expect to get attributes for 3 nodes
		ASSERT_EQ(scores_out_size, expected_scores_out_size);
		for (int i = 0; i < expected_scores_out_size; i++)
		{
			// Check that the attributes are returned in the right order
			// in respect to the ordering of the IDs
			ASSERT_EQ(scores_out[i], expected_scores_out[i]);
		}

		// Deallocate the contents of scores_out by calling C_Interface function, then
		// deallocate scores_out since we're the ones who allocated that with new. 
		DeleteScoreArray(scores_out, scores_out_size);
		delete[] scores_out;
	}

	TEST(_graphCInterface, GetNodeAttributesByIDFloat)
	{
		// Create a graph and add edges
		Graph g;
		g.addEdge(0, 1, 1); g.addEdge(0, 2, 1); g.addEdge(1, 3, 1); g.addEdge(1, 4, 1);
		g.addEdge(2, 4, 1); g.addEdge(3, 5, 1); g.addEdge(3, 6, 1); g.addEdge(4, 5, 1);
		g.addEdge(5, 6, 1); g.addEdge(5, 7, 1); g.addEdge(5, 8, 1); g.addEdge(4, 8, 1);
		g.addEdge(6, 7, 1);	g.addEdge(7, 8, 1);

		// Create a vector of node IDs and their corresponding values for our attribute
		std::vector<int> ids = { 1, 2, 5, 7, 8 };
		std::string attr = "testattribute";
		const float scores[5] = { 1.0, 2.0, 3.0, 4.0, 5.0 };
		AddNodeAttributesFloat(&g, ids.data(), attr.c_str(), scores, ids.size());

		std::vector<int> subset_ids = { 1, 5, 7 };
		std::vector<float> expected_scores_out = { 1.0, 3.0, 4.0 };
		int expected_scores_out_size = 3;
		float* scores_out = new float[5];
		int scores_out_size = 0;

		GetNodeAttributesByIDFloat(&g, subset_ids.data(), attr.c_str(), subset_ids.size(), scores_out, &scores_out_size);

		ASSERT_EQ(scores_out_size, expected_scores_out_size);

		for (int i = 0; i < scores_out_size; i++)
		{
			ASSERT_EQ(scores_out[i], expected_scores_out[i]);
		}

	}

	TEST(_graphCInterface, GetEdgeCosts) {
		HF::SpatialStructures::Graph g;
		g.Compress();

		std::string cost_type = "TestCost";

		g.addEdge(0, 1, 50.0f); g.addEdge(0, 2, 10.0f); g.addEdge(1, 2, 150.0f); g.addEdge(1, 3, 70.0f);
		g.addEdge(2, 3, 70.0f);

		g.addEdge(0, 1, 100.0f, cost_type); g.addEdge(0, 2, 50.0f, cost_type); g.addEdge(1, 2, 20.0f, cost_type);
		g.addEdge(1, 3, 1000.0f, cost_type); g.addEdge(2, 3, 1500.0f, cost_type);

		float* scores_out = new float[5];
		int scores_out_size = 0;

		GetEdgeCosts(&g, cost_type.c_str(), scores_out, &scores_out_size);

		int expected_scores_out_size = 5;
		std::vector<float> expected_scores_out = { 100.0f, 50.0f, 20.0f, 1000.0f, 1500.0f };
		ASSERT_EQ(scores_out_size, expected_scores_out_size);

		for (int i = 0; i < scores_out_size; i++) {
			ASSERT_EQ(scores_out[i], expected_scores_out[i]);
		}
		delete[] scores_out;
	}
	TEST(_graphCInterface, GetEdgeCostsFromNodeIDs) {
		HF::SpatialStructures::Graph g;
		g.Compress();

		std::string cost_type = "TestCost";

		g.addEdge(0, 1, 50.0f); g.addEdge(0, 2, 10.0f); g.addEdge(1, 2, 150.0f); g.addEdge(1, 3, 70.0f);
		g.addEdge(2, 3, 70.0f);

		g.addEdge(0, 1, 100.0f, cost_type); g.addEdge(0, 2, 50.0f, cost_type); g.addEdge(1, 2, 20.0f, cost_type);
		g.addEdge(1, 3, 1000.0f, cost_type); g.addEdge(2, 3, 1500.0f, cost_type);

		float* scores_out = new float[3];
		int scores_out_size = 0;

		std::vector<int> ids = { 0,1,1,2,2,3 };

		GetEdgeCostsFromNodeIDs(&g, ids.data(), cost_type.c_str(), ids.size(), scores_out, &scores_out_size);

		int expected_scores_out_size = 3;
		std::vector<float> expected_scores_out = { 100.0f, 20.0f, 1500.0f };

		ASSERT_EQ(scores_out_size, expected_scores_out_size);
		for (int i = 0; i < scores_out_size; i++)
		{
			ASSERT_EQ(scores_out[i], expected_scores_out[i]);
		}

		delete[] scores_out;
	}
	// Verify that deallocating the scores array doesn't corrupt the heap. 
	// The other test cases cover things like Adding and getting node attributes.
	TEST(_graphCInterface, DeleteScoreArray) {
		Graph g;
		g.addEdge(0, 1, 1); g.addEdge(0, 2, 1);	g.addEdge(1, 3, 1);	g.addEdge(1, 4, 1);
		g.addEdge(2, 4, 1);	g.addEdge(3, 5, 1);	g.addEdge(3, 6, 1);	g.addEdge(4, 5, 1);
		g.addEdge(5, 6, 1);	g.addEdge(5, 7, 1);	g.addEdge(5, 8, 1);	g.addEdge(4, 8, 1);
		g.addEdge(6, 7, 1);	g.addEdge(7, 8, 1);

		std::vector<int> ids{ 1, 3, 5, 7 };
		std::string attr_type = "cross slope";
		const char* scores[4] = { "1.4", "2.0", "2.8", "4.0" };

		AddNodeAttributes(&g, ids.data(), attr_type.c_str(), scores, ids.size());

		char** scores_out = new char* [g.size()];
		int scores_out_size = 0;
		GetNodeAttributes(&g, attr_type.c_str(), scores_out, &scores_out_size);

		DeleteScoreArray(scores_out, scores_out_size);
		delete[] scores_out;
	}

	TEST(_graphCInterface, ClearAttributeType) {
		// Create a graph and add some edges.
		Graph g;
		g.addEdge(0, 1, 1);	g.addEdge(0, 2, 1);	g.addEdge(1, 3, 1);	g.addEdge(1, 4, 1);
		g.addEdge(2, 4, 1);	g.addEdge(3, 5, 1);	g.addEdge(3, 6, 1);	g.addEdge(4, 5, 1);
		g.addEdge(5, 6, 1);	g.addEdge(5, 7, 1);	g.addEdge(5, 8, 1);	g.addEdge(4, 8, 1);
		g.addEdge(6, 7, 1);	g.addEdge(7, 8, 1);

		// Create score arrays, then assign them to the graph
		std::vector<int> ids{ 1, 3, 5, 7 };
		std::string attr_type = "cross slope";
		const char* scores[4] = { "1.4", "2.0", "2.8", "4.0" };
		AddNodeAttributes(&g, ids.data(), attr_type.c_str(), scores, ids.size());
		
		// Clear the attribute type and capture the error code
		auto res = ClearAttributeType(&g, attr_type.c_str());
		
		// Assert that the error code is successful, since we're meeting all of
		// the function's preconditions
		ASSERT_EQ(HF_STATUS::OK, res);

		// Assert that the attribute type is actually deleted
		ASSERT_EQ(g.GetNodeAttributes(attr_type).size(), 0);
	}

		TEST(_NodeCInterface, GetAllNodesFromGraph) {
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

			AddEdgeFromNodes(g, n0, n1, 1,"\0");
			AddEdgeFromNodes(g, n0, n2, 2,"\0");
			AddEdgeFromNodes(g, n1, n0, 3,"\0");
			AddEdgeFromNodes(g, n1, n2, 4,"\0");
			AddEdgeFromNodes(g, n2, n0, 5,"\0");
			AddEdgeFromNodes(g, n2, n1, 6,"\0");

			auto out_vec = new std::vector<HF::SpatialStructures::Node>;
			HF::SpatialStructures::Node* out_data = nullptr;

			GetAllNodesFromGraph(g, &out_vec, &out_data);

			DestroyGraph(g);
		}

		TEST(_NodeCInterface, GetSizeOfNodeVector) {
			// Requires #include "node.h", #include <vector>

			HF::SpatialStructures::Node n0(0, 0, 0);
			HF::SpatialStructures::Node n1(0, 1, 1);
			HF::SpatialStructures::Node n2(0, 1, 2);
			HF::SpatialStructures::Node n3(1, 2, 3);

			auto node_vec = new std::vector<HF::SpatialStructures::Node>{ n0, n1, n2, n3 };

			int node_vec_size = -1;
			GetSizeOfNodeVector(node_vec, &node_vec_size);

			DestroyNodes(node_vec);
		}

		TEST(_NodeCInterface, AggregateCosts) {
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

			AddEdgeFromNodes(g, n0, n1, 1,"\0");
			AddEdgeFromNodes(g, n0, n2, 2,"\0");
			AddEdgeFromNodes(g, n1, n0, 3,"\0");
			AddEdgeFromNodes(g, n1, n2, 4,"\0");
			AddEdgeFromNodes(g, n2, n0, 5,"\0");
			AddEdgeFromNodes(g, n2, n1, 6,"\0");

			std::vector<float>* out_vector = nullptr;
			float* out_data = nullptr;

			int aggregation_type = 0;
			AggregateCosts(g, aggregation_type, false,"\0", &out_vector, &out_data);

			DestroyGraph(g);
		}

		TEST(_NodeCInterface, CreateGraph) {
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
		}

		TEST(_NodeCInterface, AddEdgeFromNodes) {
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

			AddEdgeFromNodes(g, n0, n1, distance,"\0");

			// Release memory for g after use
			DestroyGraph(g);
		}

		TEST(_NodeCInterface, AddEdgeFromNodeIDs) {
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

			AddEdgeFromNodeIDs(g, id0, id1, distance, "");

			// Release memory for g after use
			DestroyGraph(g);
		}

		TEST(_NodeCInterface, GetCSRPointers) {
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

			AddEdgeFromNodes(g, n0, n1, 1,"\0");
			AddEdgeFromNodes(g, n0, n2, 2,"\0");
			AddEdgeFromNodes(g, n1, n0, 3,"\0");
			AddEdgeFromNodes(g, n1, n2, 4,"\0");
			AddEdgeFromNodes(g, n2, n0, 5,"\0");
			AddEdgeFromNodes(g, n2, n1, 6,"\0");

			Compress(g);

			// data = { 1, 2, 3, 4, 5, 6 }
			// r = { 0, 2, 4 }
			// c = { 1, 2, 0, 2, 0, 1 }

			// Retrieve the CSR from the graph
			CSRPtrs csr;
			GetCSRPointers(
				g, 
				&csr.nnz,
				&csr.rows, 
				&csr.cols,
				&csr.data, 
				&csr.inner_indices,
				&csr.outer_indices,
				""
			);

			// Release memory for g after use
			DestroyGraph(g);
		}

		TEST(_NodeCInterface, GetNodeID) {
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

			AddEdgeFromNodes(g, n0, n1, distance,"\0");

			float point[] = { 0, 1, 2 };
			int result_id = -1;

			GetNodeID(g, point, &result_id);

			// Release memory for g after use
			DestroyGraph(g);
		}

		TEST(_NodeCInterface, Compress) {
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

			AddEdgeFromNodes(g, n0, n1, 1,"\0");
			AddEdgeFromNodes(g, n0, n2, 2,"\0");
			AddEdgeFromNodes(g, n1, n0, 3,"\0");
			AddEdgeFromNodes(g, n1, n2, 4,"\0");
			AddEdgeFromNodes(g, n2, n0, 5,"\0");
			AddEdgeFromNodes(g, n2, n1, 6,"\0");

			Compress(g);

			// data = { 1, 2, 3, 4, 5, 6 }
			// r = { 0, 2, 4 }
			// c = { 1, 2, 0, 2, 0, 1 }

			// Release memory for g after use
			DestroyGraph(g);
		}

		TEST(_NodeCInterface, ClearGraph) {
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

			AddEdgeFromNodes(g, n0, n1, distance,"\0");

			ClearGraph(g, "");

			// Release memory for g after use
			DestroyGraph(g);
		}

		TEST(_NodeCInterface, DestroyNodes) {
			// Requires #include "node.h", #include <vector>

			HF::SpatialStructures::Node n0(0, 0, 0);
			HF::SpatialStructures::Node n1(0, 1, 1);
			HF::SpatialStructures::Node n2(0, 1, 2);
			HF::SpatialStructures::Node n3(1, 2, 3);

			auto node_vec = new std::vector<HF::SpatialStructures::Node>{ n0, n1, n2, n3 };

			// Use node_vec

			DestroyNodes(node_vec);
		}

		TEST(_NodeCInterface, DestroyEdges) {
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
		}

		TEST(_NodeCInterface, DestroyGraph) {
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
		}
	}
