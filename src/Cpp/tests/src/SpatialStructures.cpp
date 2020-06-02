#include "gtest/gtest.h"
#include <robin_hood.h>
#include <graph.h>
#include <node.h>
#include <edge.h>

using namespace HF::SpatialStructures;
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

	TEST(_Graph, AggregateCosts) {
		HF::SpatialStructures::Graph g;

		HF::SpatialStructures::Node N1(39, 39, 39);
		HF::SpatialStructures::Node N2(54, 54, 54);

		g.addEdge(N1, N2, 30);
		g.Compress();
		ASSERT_EQ(g.AggregateGraph(COST_AGGREGATE::SUM)[0], 30);
		ASSERT_EQ(g.AggregateGraph(COST_AGGREGATE::SUM)[1], 0);
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
        Node N1(0, 0, 0.0001);
        Node N2(0, 0, 0);

        ASSERT_TRUE(N1 != N2);
    }


}