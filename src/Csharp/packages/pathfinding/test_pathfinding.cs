using HumanFactors.Pathfinding;
using HumanFactors.SpatialStructures;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.IO;
using System.Runtime.InteropServices;

namespace HumanFactors.Tests.Pathfinding
{
    [TestClass]
    public class TestPathFinding
    {
        [TestMethod]
        public void ShortestPath()
        {
            Graph g = new Graph();
            g.AddEdge(0, 0, 100);
            g.AddEdge(0, 2, 50);
            g.AddEdge(1, 3, 10);
            g.AddEdge(2, 3, 10);
            g.CompressToCSR();

            var sp = HumanFactors.Pathfinding.ShortestPath.DijkstraShortestPath(g, 0, 3);
            Assert.IsTrue(sp != null);
            Span<PathMember> arr = sp.array;

            Assert.AreEqual(arr[0].cost_to_next, 50);
            Assert.AreEqual(arr[0].id, 0);

            Assert.AreEqual(arr[1].cost_to_next, 10);
            Assert.AreEqual(arr[1].id, 2);
        }

        [TestMethod]
        public void MultipleShortestPaths()
        {
            Graph g = new Graph();
            g.AddEdge(0, 0, 100);
            g.AddEdge(0, 2, 50);
            g.AddEdge(1, 3, 10);
            g.AddEdge(2, 3, 10);
            g.CompressToCSR();

            int[] start_array = { 0, 0, 0, 0, 0, 0 };
            int[] end_array = { 3, 3, 3, 3, 3, 3 };

            var short_paths = HumanFactors.Pathfinding.ShortestPath.DijkstraShortestPathMulti(g, start_array, end_array);
            foreach (var sp in short_paths)
            {
                Span<PathMember> arr = sp.array;

                Assert.AreEqual(arr[0].cost_to_next, 50);
                Assert.AreEqual(arr[0].id, 0);

                Assert.AreEqual(arr[1].cost_to_next, 10);
                Assert.AreEqual(arr[1].id, 2);
            }
        }

        [TestMethod]
        public void ShortestPathNodes()
        {
            Graph g = new Graph();
            Vector3D node0 = new Vector3D(0, 0, 1);
            Vector3D node1 = new Vector3D(0, 0, 2);
            Vector3D node2 = new Vector3D(0, 0, 3);
            Vector3D node3 = new Vector3D(0, 0, 4);

            g.AddEdge(node0, node0, 100);
            g.AddEdge(node0, node2, 50);
            g.AddEdge(node1, node3, 10);
            g.AddEdge(node2, node3, 10);
            g.CompressToCSR();

            int[] ids = {
                g.GetNodeID(node0),
                g.GetNodeID(node1),
                g.GetNodeID(node2),
                g.GetNodeID(node3)
            };

            var sp = HumanFactors.Pathfinding.ShortestPath.DijkstraShortestPath(g, node0, node3);
            Assert.IsTrue(sp != null);
            Span<PathMember> arr = sp.array;

            Assert.AreEqual(arr[0].cost_to_next, 50);
            Assert.AreEqual(arr[0].id, ids[0]);

            Assert.AreEqual(arr[1].cost_to_next, 10);
            Assert.AreEqual(arr[1].id, ids[2]);
        }

        [TestMethod]
        public void MultipleShortestPathsNodes()
        {
            Graph g = new Graph();
            Vector3D node0 = new Vector3D(0, 0, 1);
            Vector3D node1 = new Vector3D(0, 0, 2);
            Vector3D node2 = new Vector3D(0, 0, 3);
            Vector3D node3 = new Vector3D(0, 0, 4);

            g.AddEdge(node0, node0, 100);
            g.AddEdge(node0, node2, 50);
            g.AddEdge(node1, node3, 10);
            g.AddEdge(node2, node3, 10);
            g.CompressToCSR();

            // Ids assigned to these nodes by the graph.
            int[] ids = {
                g.GetNodeID(node0),
                g.GetNodeID(node1),
                g.GetNodeID(node2),
                g.GetNodeID(node3)
            };

            Vector3D[] start_array = { node0, node0, node0, node0, node0, node0};
            Vector3D[] end_array = { node3, node3, node3, node3, node3, node3 };

            var short_paths = HumanFactors.Pathfinding.ShortestPath.DijkstraShortestPathMulti(g, start_array, end_array);
            foreach (var sp in short_paths)
            {
                Span<PathMember> arr = sp.array;

                Assert.AreEqual(arr[0].cost_to_next, 50);
                Assert.AreEqual(arr[0].id, ids[0]);

                Assert.AreEqual(arr[1].cost_to_next, 10);
                Assert.AreEqual(arr[1].id, ids[2]);
            }
        }
    }
}