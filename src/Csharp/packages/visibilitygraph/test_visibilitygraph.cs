using DHARTAPI.Geometry;
using DHARTAPI.RayTracing;
using DHARTAPI.SpatialStructures;
using DHARTAPI.VisibilityGraph;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace DHARTAPI.Tests.VisibilityGraph
{
    [TestClass]
    public class VisibilityGraphTest
    {
        private bool IsNear(float actual, float expected) => Math.Abs(actual - expected) < 0.001f;

        private const string good_mesh_path = "ExampleModels/plane.obj";
        private static Vector3D Point1 = new Vector3D(0, 0, 0);
        private static Vector3D Point2 = new Vector3D(0, 1, 0);
        private static Vector3D Point3 = new Vector3D(1, 0, 0);
        private static Vector3D Point4 = new Vector3D(1, 0, -5);
        private static Vector3D[] nodes = { Point1, Point2, Point3, Point4 };
        private static Vector3D[] group_a = { Point1, Point2 };
        private static Vector3D[] group_b = { Point3, Point4 };

        private EmbreeBVH GetBVH(string path = good_mesh_path)
        {
		    //! [CreateBVHAndDefineNodes]
            // Load the example plane and convert it to z-up
            MeshInfo MI = (OBJLoader.LoadOBJ("ExampleModels/plane.obj", CommonRotations.Yup_To_Zup));

            // Construct a BVH from this mesh
            EmbreeBVH bvh = new EmbreeBVH(MI);

            // Define all nodes and put them into an array,
			Vector3D Point1 = new Vector3D(0, 0, 0);
			Vector3D Point2 = new Vector3D(0, 1, 0);
			Vector3D Point3 = new Vector3D(1, 0, 0);
			Vector3D Point4 = new Vector3D(1, 0, -4); // Node 4 is beneath the plane so it should have no connections
			Vector3D[] nodes = { Point1, Point2, Point3, Point4 };
		    //! [CreateBVHAndDefineNodes]

            return bvh;
        }

		private bool IsValidGraph(Graph G)
		{
			if (G == null) return false;

			var nodes = G.getNodes();

			// If the graph has no nodes, something is wrong
			if (!(nodes.array.Length > 0))
				return false;

			//  Check that all the nodes are finite
			for (int i = 0; i < nodes.size; i++)
			{
				var n = nodes[i];
				if (!n.x.IsFinite() || !n.y.IsFinite() || !n.z.IsFinite())
					return false;
			}

			return true;

		}

		[TestMethod]
        public void Directed()
        {
            var bvh = GetBVH();

            //! [EX_DirectedVisibilityGraph]
            
            // Create a visibility from every node to every other node
            Graph G = DHARTAPI.VisibilityGraph.VisibilityGraph.GenerateAllToAll(bvh, nodes);

            // Aggregate the graph's edges to get the total distance from this node to all of its edges
            var results = G.AggregateEdgeCosts(GraphEdgeAggregation.SUM);

            // Print output
            Debug.WriteLine(results);

            //! [EX_DirectedVisibilityGraph]
            Assert.IsTrue(IsValidGraph(G));

            // Check outputs against expectations
            Assert.IsTrue(IsNear(2.0f, results[0]));
            Assert.IsTrue(IsNear(2.4142f, results[1]));
            Assert.IsTrue(IsNear(2.4142f, results[2]));
            Assert.IsTrue(IsNear(0, results[3]));
        }

        [TestMethod]
        public void Undirected()
        {
            var bvh = GetBVH();
            Graph G = DHARTAPI.VisibilityGraph.VisibilityGraph.GenerateAllToAll(bvh, nodes, directed:false);
            Assert.IsTrue(IsValidGraph(G));
        }

        [TestMethod]
        public void GroupToGroup()
        {
            var bvh = GetBVH();

            //! [EX_GroupToGroup]

            // Define two groups of nodes
		    Vector3D[] group_a = { Point1, Point2 };
		    Vector3D[] group_b = { Point3, Point4 };

            // Generate a visibility graph from group A to group B
            Graph G = DHARTAPI.VisibilityGraph.VisibilityGraph.GenerateGroupToGroup(bvh, group_a, group_b);

            // Print the results
            var results = G.AggregateEdgeCosts(GraphEdgeAggregation.SUM);

            Debug.WriteLine("[{0}, {1}, {2}, {3}]", results[0], results[1], results[2], results[3]);

            //! [EX_GroupToGroup]
            // Assert this is a valid graph
            Assert.IsTrue(IsValidGraph(G));

            // Check outputs against expectations
            Assert.IsTrue(IsNear(1.0f, results[0]));
            Assert.IsTrue(IsNear(1.4142f, results[1]));
            Assert.IsTrue(IsNear(0, results[2]));
            Assert.IsTrue(IsNear(0, results[3]));

            //! [EX_GroupToGroupUndirected]

            // Print the undirected results
            results = G.AggregateEdgeCosts(GraphEdgeAggregation.SUM, directed:false);

            Debug.WriteLine("[{0}, {1}, {2}, {3}]", results[0], results[1], results[2], results[3]);

            //! [EX_GroupToGroupUndirected]

        }
    }
}