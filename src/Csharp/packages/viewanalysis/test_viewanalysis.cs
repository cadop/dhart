using Microsoft.VisualStudio.TestTools.UnitTesting;
using HumanFactors.Geometry;
using HumanFactors.SpatialStructures;
using HumanFactors.RayTracing;
using System.Runtime.InteropServices;
using System;
using System.IO;
using HumanFactors.ViewAnalysis;

namespace HumanFactors.Tests.ViewAnalysis
{

    [TestClass]
    public class ViewAnalysis
    {
        private const string good_mesh_path = "ExampleModels/plane.obj";
        private static Vector3D Point = new Vector3D(0, 0, 0);
        private static Vector3D[] nodes = { Point };
        private const int num_rays = 1000;
        
        private EmbreeBVH GetBVH(string path = good_mesh_path)
        {
            EmbreeBVH bvh = new EmbreeBVH(HumanFactors.Geometry.OBJLoader.LoadOBJ(path, HumanFactors.Geometry.CommonRotations.Yup_To_Zup));

            return bvh;
        }
        
        [TestMethod]
        public void AggregateSucceed()
        {
            var bvh = GetBVH();

            var results = HumanFactors.ViewAnalysis.ViewAnalysis.ViewAnalysisAggregate(bvh, nodes, num_rays, type:ViewAggregateType.COUNT);
            var arr = results.array;

            Assert.AreEqual(1, arr.Length);
            Assert.IsTrue(arr[0] > 0);
        }

        [TestMethod]
        public void NonAggregateSucceed()
        {
            var bvh = GetBVH();

            var results = HumanFactors.ViewAnalysis.ViewAnalysis.ViewAnalysisStandard(bvh, nodes, num_rays);
            var arr = results.array;

            Assert.IsTrue(arr.Length > 0);
            Assert.IsTrue(results[0].Length > 0);
        }
        [TestMethod]
        public void SphereDistributeRays()
        {
            var bvh = GetBVH();

            var results = HumanFactors.ViewAnalysis.ViewAnalysis.SphericallyDistributeRays(1000);
            var arr = results.array;

            var direction = results[0];
            Assert.AreEqual(direction.Length, 3);
        }
    }
}