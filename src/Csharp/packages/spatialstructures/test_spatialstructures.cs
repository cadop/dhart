using Microsoft.VisualStudio.TestTools.UnitTesting;
using HumanFactors.SpatialStructures;
using System.Linq;

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
            Assert.AreEqual(score_arr[0], 39);
            Assert.AreEqual(score_arr[1], 0);
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
        public void CalculateAndStoreCrossSlope()
        {
            // Create the graph
            Graph g = new Graph();

            // Create 7 nodes
            Vector3D n0 = new Vector3D(2, 6, 6);
            Vector3D n1 = new Vector3D(0, 0, 0);
            Vector3D n2 = new Vector3D(-5, 5, 4);
            Vector3D n3 = new Vector3D(-1, 1, 1);
            Vector3D n4 = new Vector3D(2, 2, 2);
            Vector3D n5 = new Vector3D(5, 3, 2);
            Vector3D n6 = new Vector3D(-2, -5, 1);

            // Add 9 edges
            g.AddEdge(n0, n1, 0); // [ -2, -6, -6 ]
            g.AddEdge(n1, n2, 0); // [ -5,  5,  4 ]
            g.AddEdge(n1, n3, 0); // [ -1,  1,  1 ]
            g.AddEdge(n1, n4, 0); // [  2,  2,  2 ]
            g.AddEdge(n2, n4, 0); // [ -9, -3, -2 ]
            g.AddEdge(n3, n5, 0); // [ -6,  2,  1 ]
            g.AddEdge(n5, n6, 0); // [ -7, -8, -1 ]
            g.AddEdge(n4, n6, 0); // [ -6, -7, -1 ]

            // Compress the graph after adding edges
            g.CompressToCSR();

            // Calculate and store edge type in g: cross slope
            CostAlgorithms.CalculateAndStoreCrossSlope(g);
        }
    }
}
 