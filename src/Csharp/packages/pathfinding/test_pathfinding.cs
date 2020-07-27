using HumanFactors.Pathfinding;
using HumanFactors.SpatialStructures;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.IO;
using System.Runtime.InteropServices;
using HumanFactors.Pathfinding;
using System.Collections.Generic;

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
        public void ShortestPath_Cost()
        {
            string test_cost = "test";
            // Create a new graph with nodes and edges 
            Graph g = new Graph();
            Vector3D node0 = new Vector3D(0, 0, 1);
            Vector3D node1 = new Vector3D(0, 0, 2);
            Vector3D node2 = new Vector3D(0, 0, 3);
            Vector3D node3 = new Vector3D(0, 0, 4);
            g.AddEdge(node0, node0, 100);
            g.AddEdge(node0, node2, 50);
            g.AddEdge(node1, node3, 10);
            g.AddEdge(node2, node3, 10);

            // Assert that no cost is found if I try to create
            // A path with a cost type that doesn't exist
            try { 
                HumanFactors.Pathfinding.ShortestPath.DijkstraShortestPath(
                    g, node0, node3, "CostThatDoesn'tExist"        
                ); 
            }
            catch (KeyNotFoundException) { };

            // Compress the graph and add nodes to the alternate cost
            g.CompressToCSR();
            g.AddEdge(node0, node0, 100, test_cost);
            g.AddEdge(node0, node2, 50, test_cost);
            g.AddEdge(node1, node3, 10, test_cost);
            g.AddEdge(node2, node3, 10, test_cost);

            HumanFactors.Pathfinding.ShortestPath.DijkstraShortestPath(
                g, node0, node3, test_cost
            );
            
            // create a path using the default cost, and a custom cost
            var sp = HumanFactors.Pathfinding.ShortestPath.DijkstraShortestPath(g, node0, node3);
            var sp_cost = HumanFactors.Pathfinding.ShortestPath.DijkstraShortestPath(g, node0, node3, test_cost);

            // Assert that the path was created successfully, and that it is equal to the default cost
            Assert.IsTrue(sp_cost != null, "Path with a custom cost type couldn't be created.");
            Assert.IsTrue(sp.Equals(sp_cost), "Paths with custom cost types do not equal paths with default cost types");
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


        [TestMethod]
        public void MultipleShortestPaths_cost()
        {
            string test_cost = "test";

            Graph g = new Graph();
            Vector3D node0 = new Vector3D(0, 0, 1);
            Vector3D node1 = new Vector3D(0, 0, 2);
            Vector3D node2 = new Vector3D(0, 0, 3);
            Vector3D node3 = new Vector3D(0, 0, 4);

            g.AddEdge(node0, node0, 0);
            g.AddEdge(node0, node2, 0);
            g.AddEdge(node1, node3, 0);
            g.AddEdge(node2, node3, 0);
            g.CompressToCSR();

            // Ids assigned to these nodes by the graph.
            int[] ids = {
                g.GetNodeID(node0),
                g.GetNodeID(node1),
                g.GetNodeID(node2),
                g.GetNodeID(node3)
            };
           
            g.AddEdge(node0, node0, 100, test_cost);
            g.AddEdge(node0, node2, 50, test_cost);
            g.AddEdge(node1, node3, 10, test_cost);
            g.AddEdge(node2, node3, 10, test_cost);

            // Assert that no cost is found if I try to create
            // A path with a cost type that doesn't exist

            Vector3D[] start_array = { node0, node0, node0, node0, node0, node0 };
            Vector3D[] end_array = { node3, node3, node3, node3, node3, node3 };
            int[] start_int_array = {ids[0], ids[0], ids[0], ids[0], ids[0], ids[0] };
            int[] end_int_array = {ids[3], ids[3], ids[3], ids[3], ids[3], ids[3] };

            // Ensure we throw if we give it a bad cost type
            bool did_throw = false;
            try
            {
                HumanFactors.Pathfinding.ShortestPath.DijkstraShortestPathMulti(
                    g, start_array, end_array, "CostThatDoesn'tExist"
                );
            }
            catch (KeyNotFoundException) { did_throw = true; };

            Assert.IsTrue(did_throw, "Giving the pathfinder an invalid cost type didn't throw an exception.");

            // Position Overload
            var short_paths = HumanFactors.Pathfinding.ShortestPath.DijkstraShortestPathMulti(
                g, 
                start_array, 
                end_array, 
                test_cost
            );
            // ID Overload
            var short_int_paths = HumanFactors.Pathfinding.ShortestPath.DijkstraShortestPathMulti(
                g, 
                start_int_array, 
                end_int_array, 
                test_cost
            );

            // Assert that every path matches our expectations
            for( int i = 0; i < short_paths.Length; i++)
            {
                Span<PathMember> arr = short_paths[i].array;

                Assert.AreEqual(arr[0].cost_to_next, 50);
                Assert.AreEqual(arr[0].id, ids[0]);

                Assert.AreEqual(arr[1].cost_to_next, 10);
                Assert.AreEqual(arr[1].id, ids[2]);


                // Assert that this path equals the integer version
                Assert.IsTrue(short_paths[i].Equals(short_int_paths[i]));
            }
        }
    }
}