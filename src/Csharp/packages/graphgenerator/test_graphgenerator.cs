using Microsoft.VisualStudio.TestTools.UnitTesting;
using HumanFactors.Geometry;
using HumanFactors.SpatialStructures;
using HumanFactors.RayTracing;
using System.Runtime.InteropServices;

namespace HumanFactors.Tests.GraphGenerator
{
    [TestClass]
    public class GraphGenerator
    {
        private const string plane_path = "ExampleModels/plane.obj";
        [TestMethod]
        public void ValidGenerationIsNotNull()
        {
            MeshInfo Mesh = OBJLoader.LoadOBJ(plane_path, CommonRotations.Yup_To_Zup);
            EmbreeBVH BVH = new EmbreeBVH(Mesh);

            Vector3D start_point = new Vector3D(0, 0, 1);
            Vector3D spacing = new Vector3D(1, 1, 1);

            Graph G = HumanFactors.GraphGenerator.GraphGenerator.GenerateGraph(BVH, start_point, spacing, 999999);
            
            Assert.IsNotNull(G);

            // Just to be extra sure
            var pts = G.getNodes();

            Assert.AreNotEqual(pts.size, 0);
        }

        [TestMethod]
        public void InvalidGenerationIsCaught()
        {
            MeshInfo Mesh = OBJLoader.LoadOBJ(plane_path, CommonRotations.Yup_To_Zup);
            EmbreeBVH BVH = new EmbreeBVH(Mesh);

            Vector3D start_point = new Vector3D(0, 9000, 1);
            Vector3D spacing = new Vector3D(1, 1, 1);

            Graph G = HumanFactors.GraphGenerator.GraphGenerator.GenerateGraph(BVH, start_point, spacing, 999999);

            Assert.IsNull(G);
        }

    }
}