using Microsoft.VisualStudio.TestTools.UnitTesting;
using DHARTAPI.Geometry;
using DHARTAPI.SpatialStructures;
using DHARTAPI.RayTracing;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace DHARTAPI.Tests.GraphGenerator
{
    [TestClass]
    public class GraphGenerator
    {
        private const string plane_path = "ExampleModels/plane.obj";
        private const string obstacle_plane_path = "ExampleModels/obstacle_plane.obj";
        private const string step_path = "ExampleModels/step_0_5.obj";
        [TestMethod]
        public void ValidGenerationIsNotNull()
        {
            MeshInfo Mesh = OBJLoader.LoadOBJ(plane_path, CommonRotations.Yup_To_Zup);
            EmbreeBVH BVH = new EmbreeBVH(Mesh);

            Vector3D start_point = new Vector3D(0, 0, 1);
            Vector3D spacing = new Vector3D(1, 1, 1);

            Graph G = DHARTAPI.GraphGenerator.GraphGenerator.GenerateGraph(BVH, start_point, spacing, 999999);
            
            Assert.IsNotNull(G);

            // Just to be extra sure
            var pts = G.getNodes();

            Assert.AreNotEqual(pts.size, 0);
        }

        [TestMethod]
        public void LargeNumberDoesntCauseStackOverflow()
        {
            MeshInfo Mesh = OBJLoader.LoadOBJ(plane_path, CommonRotations.Yup_To_Zup);
            EmbreeBVH BVH = new EmbreeBVH(Mesh);

            Vector3D start_point = new Vector3D(0.001f, 0.001f, 1);
            Vector3D spacing = new Vector3D(0.0001f, 0.0001f, 1.7f);

            Graph G = DHARTAPI.GraphGenerator.GraphGenerator.GenerateGraph(BVH, start_point, spacing, 1000000);

            Assert.IsNotNull(G);

            Debug.WriteLine(G.NumNodes());

            float[] attrs = new float[G.NumNodes()];
            for(int i = 0; i < G.NumNodes(); i++)
                attrs[i] = i;

            G.AddNodeAttribute("node_attr", attrs);
            Assert.AreEqual(G.GetNodeAttributes("node_attr").Length, attrs.Length);

        }

        [TestMethod]
        public void InvalidGenerationIsCaught()
        {
            MeshInfo Mesh = OBJLoader.LoadOBJ(plane_path, CommonRotations.Yup_To_Zup);
            EmbreeBVH BVH = new EmbreeBVH(Mesh);

            Vector3D start_point = new Vector3D(0, 9000, 1);
            Vector3D spacing = new Vector3D(1, 1, 1);

            Graph G = DHARTAPI.GraphGenerator.GraphGenerator.GenerateGraph(BVH, start_point, spacing, 999999);

            Assert.IsNull(G);
        }

        [TestMethod]
        public void ObstacleSupport()
        {
            MeshInfo[] Meshes = OBJLoader.LoadOBJSubmeshes(
                obstacle_plane_path,
                GROUP_METHOD.BY_GROUP,
                CommonRotations.Yup_To_Zup[0],
                CommonRotations.Yup_To_Zup[1],
                CommonRotations.Yup_To_Zup[2]
           );

            EmbreeBVH BVH = new EmbreeBVH(Meshes);

            Vector3D start_point = new Vector3D(0, 0, 1);
            Vector3D spacing = new Vector3D(0.25f, 0.25f, 1);

            Graph obstacle_graph = DHARTAPI.GraphGenerator.GraphGenerator.GenerateGraph(
                BVH, start_point, spacing,
                obstacle_ids: new int[] { Meshes[1].id }
            );

            Graph non_obstacle_graph = DHARTAPI.GraphGenerator.GraphGenerator.GenerateGraph(
                BVH, start_point, spacing
            );

            int num_obs_nodes = obstacle_graph.NumNodes();
            int num_non_nodes = non_obstacle_graph.NumNodes();

            Assert.AreNotEqual(num_obs_nodes, num_non_nodes, "Obstacles produced identical output to non-obstacles");
            Assert.IsTrue(num_obs_nodes < num_non_nodes, "Obstacles did not produce less output than without" );
        }

        [TestMethod]
        public void EdgeTypesContainStepTags()
        {
            MeshInfo Mesh = OBJLoader.LoadOBJ(step_path);
            EmbreeBVH BVH = new EmbreeBVH(Mesh);

            Vector3D start_point = new Vector3D(0.25f, 0.25f, 2);
            Vector3D spacing = new Vector3D(1.0f, 0.25f, 1.0f);

            Graph G = DHARTAPI.GraphGenerator.GraphGenerator.GenerateGraph(
                BVH,
                start_point,
                spacing,
                max_nodes: 500,
                up_step: 1.0f,
                down_step: 1.0f,
                store_edge_types: true
            );

            Assert.IsNotNull(G);

            CSRInfo csr = G.CompressToCSR("edge_type");
            float[] edge_types = new float[csr.nnz];
            Marshal.Copy(csr.data, edge_types, 0, csr.nnz);

            bool has_step_up = false;
            bool has_step_down = false;

            foreach (float edge_type in edge_types)
            {
                int type = (int)System.Math.Round(edge_type);
                if (type == 2)
                    has_step_up = true;
                else if (type == 3)
                    has_step_down = true;

                if (has_step_up && has_step_down)
                    break;
            }

            Assert.IsTrue(has_step_up, "Expected at least one step-up edge type.");
            Assert.IsTrue(has_step_down, "Expected at least one step-down edge type.");
        }

    }
}
