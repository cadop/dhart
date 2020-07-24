using Microsoft.VisualStudio.TestTools.UnitTesting;
using HumanFactors.SpatialStructures;
using System.Linq;
using System.Runtime.CompilerServices;
using HumanFactors.Exceptions;
using System.Collections.Generic;

namespace HumanFactors.Tests.SpatialStructures
{
    [TestClass]
    public class TestGraph
    {
        [TestMethod]
        public void CreateEmptyGraph()
        {
            Graph G = new Graph();
        }
        [TestMethod]
        public void AddEdgeFromID()
        {
            Graph G = new Graph();
            G.AddEdge(0, 1, 39);
        }
        [TestMethod]
        public void AddEdgeFromV3()
        {
            Graph G = new Graph();
            G.AddEdge(new Vector3D(0,0,2), new Vector3D(0,0,1), 39);
        }

        [TestMethod]
        public void AddEdgeFromV3WithCosts()
        {
            Graph G = new Graph();
            G.AddEdge(new Vector3D(0, 0, 2), new Vector3D(0, 0, 1), 39);
            G.CompressToCSR();
            G.AddEdge(new Vector3D(0, 0, 2), new Vector3D(0, 0, 1), 100, "Not Default Cost");
        }

        [TestMethod]
        public void GetCSRPointers()
        {
            Graph G = new Graph();
            G.AddEdge(new Vector3D(0,0,2), new Vector3D(0,0,1), 39);
            G.CompressToCSR();
        }
        
        [TestMethod]
        public void GetNodes()
        {
            Graph G = new Graph();
            G.AddEdge(new Vector3D(0,0,2), new Vector3D(0,0,1), 39);

            var nodes = G.getNodes();
            Assert.IsTrue(nodes.size == 2);
        }
        [TestMethod]
        public void GetNodesCopy()
        {
            Graph G = new Graph();
            G.AddEdge(new Vector3D(0, 0, 2), new Vector3D(0, 0, 1), 39);

            var nodes = G.getNodes();
            var arr = nodes.CopyArray();
            Assert.IsTrue(arr.Count() == 2);
        }
        [TestMethod]
        public void AggregateNodes()
        {
            Graph G = new Graph();
            G.AddEdge(new Vector3D(0,0,2), new Vector3D(0,0,1), 39);

            var scores = G.AggregateEdgeCosts(GraphEdgeAggregation.SUM);
            var score_arr = scores.array;
            Assert.AreEqual(39, score_arr[0]);
            Assert.AreEqual(0, score_arr[1]);
        }

        [TestMethod]
        public void GetIDFromNode()
        {
            Graph g = new Graph();
            Vector3D node1 = new Vector3D(0, 0, 1);
            Vector3D node2 = new Vector3D(0, 0, 2);
            Vector3D node4 = new Vector3D(0, 0, 39);
            g.AddEdge(node1, node2, 10);

            Assert.IsTrue(g.GetNodeID(node1) >= 0);
            Assert.IsTrue(g.GetNodeID(node2) >= 0);
            Assert.IsTrue(g.GetNodeID(node4) < 0);
        }

        [TestMethod]
        public void GetEdgeCost()
        {
            Graph g = new Graph();
            
            g.AddEdge(0, 1, 100);

            try {
                g.GetCost(0, 1);
            }
            catch (HumanFactors.Exceptions.LogicError) {
            }

            g.CompressToCSR();
            Assert.AreEqual(-1, g.GetCost(1, 2));
            Assert.AreEqual(100, g.GetCost(0, 1));
        }

        [TestMethod]
        public void GetEdgeCostMulti()
        {
            Graph g = new Graph();
            string cost_type = "TestCost";

            // This should throw because the user is trying
            // to add a cost type to a graph that isn't compressed
            try { g.AddEdge(0, 1, 39, cost_type);}
            catch (LogicError) {}
            
            g.CompressToCSR();

            // This should throw an exception since the edge
			// doesn't already exist in the graph for the default cost_type.
            try { g.AddEdge(0, 1, 39, cost_type);}
            catch (LogicError) {}

            // Add the edge for the default cost type
            g.AddEdge(0, 1, 54);

            // Assert that this edge doesn't exist for this alternate cost type
            try { g.GetCost(1, 2, cost_type); }
            catch (KeyNotFoundException) { };


            // Add the edge for the alternate cost type
            g.AddEdge(0, 1, 100, cost_type);

            Assert.AreEqual(-1, g.GetCost(0, 2, cost_type),
                "A cost that doesn't exist for this type returns something other than -1." 
           );
            
            // Assert that the default graph is still readable
            Assert.AreEqual(54, g.GetCost(0, 1), 
                "Adding another cost modified the cost in the default graph.");
            
            // Assert that the non-default cost is still readable
            Assert.AreEqual(100, g.GetCost(0, 1, cost_type), "The alternate cost cannot be read");
        }

    }
}