using HumanFactors.Geometry;
using HumanFactors.RayTracing;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Diagnostics;

namespace HumanFactors.Tests.RayTracing
{
    [TestClass]
    public class TestBVH
    {
        private const string good_mesh_path = "ExampleModels/plane.obj";

        [TestMethod]
        public void BVHCreation()
        {
            MeshInfo Mesh = OBJLoader.LoadOBJ(good_mesh_path, CommonRotations.Yup_To_Zup);
            EmbreeBVH BVH = new EmbreeBVH(Mesh);
        }
    }

    [TestClass]
    public class TestRayTracer
    {
        private const string plane_path = "ExampleModels/plane.obj";

        [TestMethod]
        public void IntersectForPoint()
        {
            MeshInfo Mesh = OBJLoader.LoadOBJ(plane_path, CommonRotations.Yup_To_Zup);
            EmbreeBVH BVH = new EmbreeBVH(Mesh);

            Vector3D origin = new Vector3D(0, 0, 1);
            Vector3D good_direction = new Vector3D(0, 0, -1);
            Vector3D bad_direction = new Vector3D(0, 0, 1);

            Vector3D good_point = HumanFactors.RayTracing.EmbreeRaytracer.IntersectForPoint(BVH, origin, good_direction, -1);
            Vector3D bad_point = HumanFactors.RayTracing.EmbreeRaytracer.IntersectForPoint(BVH, origin, bad_direction, -1);
            Assert.IsTrue(good_point.IsValid());
            Assert.IsTrue(!bad_point.IsValid());
        }

        [TestMethod]
        public void PointsEqualDirAndOrigin()
        {
            MeshInfo Mesh = OBJLoader.LoadOBJ(plane_path, CommonRotations.Yup_To_Zup);
            EmbreeBVH BVH = new EmbreeBVH(Mesh);

            // C# has no function/constructor to just fill an array with a value?
            Vector3D[] origin_vector = new Vector3D[100];
            Vector3D[] direction_vector = new Vector3D[100];
            for (int i = 0; i < 100; i++)
            {
                origin_vector[i] = new Vector3D(0, 0, 1);
                direction_vector[i] = new Vector3D(0, 0, -1);
            }

            Vector3D[] results = EmbreeRaytracer.IntersectForPoints(BVH, origin_vector, direction_vector, -1);

            var result_1 = results[0];
            foreach (var result in results)
                Assert.IsTrue(result_1.DistanceTo(result) < 0.001);
        }


        [TestMethod]
        public void DistanceSingleRay()
        {
            MeshInfo Mesh = OBJLoader.LoadOBJ(plane_path, CommonRotations.Yup_To_Zup);
            EmbreeBVH BVH = new EmbreeBVH(Mesh);

            // C# has no function/constructor to just fill an array with a value?
            RayResult[] results= new RayResult[100];
            Vector3D point = new Vector3D(0, 0, 1);
            Vector3D direction = new Vector3D(0, 0, -1);

            for (int i = 0; i < 100; i++)
               results[i] = EmbreeRaytracer.IntersectForDistance(BVH, point, direction, -1);

            var result_1 = results[0];
            foreach (var result in results)
                Assert.IsTrue(Math.Abs(result_1.distance - result.distance) < 0.001);
        }

        [TestMethod]
        public void PointsMultiDir()
        {
            MeshInfo Mesh = OBJLoader.LoadOBJ(plane_path, CommonRotations.Yup_To_Zup);
            EmbreeBVH BVH = new EmbreeBVH(Mesh);

            // C# has no function/constructor to just fill an array with a value?
            Vector3D[] origin_vector = new Vector3D[1];
            Vector3D[] direction_vector = new Vector3D[100];
            origin_vector[0] = new Vector3D(0, 0, 1);
            for (int i = 0; i < 100; i++)
            {
                direction_vector[i] = new Vector3D(0, 0, -1);
            }

            Vector3D[] results = EmbreeRaytracer.IntersectForPoints(BVH, origin_vector, direction_vector, -1);

            var result_1 = results[0];
            foreach (var result in results)
                Assert.IsTrue(result_1.DistanceTo(result) < 0.001);
        }

        [TestMethod]
        public void PointsMultiOrigin()
        {
            MeshInfo Mesh = OBJLoader.LoadOBJ(plane_path, CommonRotations.Yup_To_Zup);
            EmbreeBVH BVH = new EmbreeBVH(Mesh);

            // C# has no function/constructor to just fill an array with a value?
            Vector3D[] origin_vector = new Vector3D[100];
            Vector3D[] direction_vector = new Vector3D[1];
            direction_vector[0] = new Vector3D(0, 0, -1);
            for (int i = 0; i < 100; i++)
            {
                origin_vector[i] = new Vector3D(0, 0, 1);
            }

            Vector3D[] results = EmbreeRaytracer.IntersectForPoints(BVH, origin_vector, direction_vector, -1);

            var result_1 = results[0];
            foreach (var result in results)
                Assert.IsTrue(result_1.DistanceTo(result) < 0.001);
        }

        [TestMethod]
        public void MultipleRaysDistance()
        {
            MeshInfo Mesh = OBJLoader.LoadOBJ(plane_path, CommonRotations.Yup_To_Zup);
            EmbreeBVH BVH = new EmbreeBVH(Mesh);

            Vector3D[] origin_vector = new Vector3D[100];
            Vector3D[] direction_vector = new Vector3D[100];
            for (int i = 0; i < 100; i++)
            {
                origin_vector[i] = new Vector3D(0, 0, 1);
                direction_vector[i] = new Vector3D(0, 0, -1);
            }

            RayResults results = EmbreeRaytracer.IntersectForDistances(BVH, origin_vector, direction_vector, -1);
            var result_span = results.array;
            var result_base = result_span[0];

            Assert.IsTrue(!Single.IsNaN(result_base.distance));
            foreach (var result in result_span)
                Assert.IsTrue(Math.Abs(result_base.distance - result.distance) < 0.001f);
        }

        [TestMethod]
        public void OcclusionRays()
        {
            MeshInfo Mesh = OBJLoader.LoadOBJ(plane_path, CommonRotations.Yup_To_Zup);
            EmbreeBVH BVH = new EmbreeBVH(Mesh);

            Vector3D[] origin_vector = new Vector3D[100];
            Vector3D[] direction_vector = new Vector3D[100];
            for (int i = 0; i < 100; i++)
            {
                origin_vector[i] = new Vector3D(0, 0, 1);
                direction_vector[i] = new Vector3D(0, 0, -1);
            }

            bool[] results = EmbreeRaytracer.IntersectOccluded(BVH, origin_vector, direction_vector, -1);
            
            foreach (bool result in results)
                Assert.IsTrue(result);
        }
    }
}