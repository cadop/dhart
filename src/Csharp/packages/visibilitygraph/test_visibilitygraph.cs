using HumanFactors.Geometry;
using HumanFactors.RayTracing;
using HumanFactors.SpatialStructures;
using HumanFactors.VisibilityGraph;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Diagnostics;
using System.IO;
using System.Runtime.InteropServices;

namespace HumanFactors.Tests.VisibilityGraph
{
    [TestClass]
    public class VisibilityGraphTest
    {
        private const string good_mesh_path = "ExampleModels/plane.obj";
        private static Vector3D Point1 = new Vector3D(0, 0, 0);
        private static Vector3D Point2 = new Vector3D(0, 1, 0);
        private static Vector3D Point3 = new Vector3D(1, 0, 0);
        private static Vector3D Point4 = new Vector3D(1, 1, 0);
        private static Vector3D[] nodes = { Point1, Point2, Point3, Point4 };
        private static Vector3D[] group_a = { Point1, Point2 };
        private static Vector3D[] group_b = { Point3, Point4 };

        private EmbreeBVH GetBVH(string path = good_mesh_path)
        {
            EmbreeBVH bvh = new EmbreeBVH(HumanFactors.Geometry.OBJLoader.LoadOBJ(path, HumanFactors.Geometry.CommonRotations.Yup_To_Zup));

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
            Graph G = HumanFactors.VisibilityGraph.VisibilityGraph.GenerateAllToAll(bvh, nodes, directed:true);

            Assert.IsTrue(IsValidGraph(G));
        }

        [TestMethod]
        public void Undirected()
        {
            var bvh = GetBVH();
            Graph G = HumanFactors.VisibilityGraph.VisibilityGraph.GenerateAllToAll(bvh, nodes, directed:false);
            Assert.IsTrue(IsValidGraph(G));
        }

        [TestMethod]
        public void GroupToGroup()
        {
            var bvh = GetBVH();
            Graph G = HumanFactors.VisibilityGraph.VisibilityGraph.GenerateGroupToGroup(bvh, group_a, group_b);
            Assert.IsTrue(IsValidGraph(G));
        }
    }
}