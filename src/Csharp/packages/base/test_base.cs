using Microsoft.VisualStudio.TestTools.UnitTesting;

using System.Diagnostics;
using System;
using DHARTAPI.NativeUtils;
using System.Collections.Generic;
using System.Linq;
using DHARTAPI.Geometry;

namespace DHARTAPI.Tests.Base
{
    [TestClass]
    public class UnitTestTest
    {
        [TestMethod]
        public void TestTrue()
        {
            Assert.IsTrue(true);
        }
    
       }

    [TestClass]
    public class OcclusionPerformance
        /// Compare calling intersectOccluded with a float array (which then has its own
        /// overload for C_CastOcclusionRays) vs. using IntersectOccluded with a Vector3D array
    {
        private const string plane_path = "ExampleModels/plane.obj";

        [TestMethod]
        public void VectorThreeD()
        {
            var stopwatch = new Stopwatch();

            MeshInfo Mesh = OBJLoader.LoadOBJ(plane_path, CommonRotations.Yup_To_Zup);
            DHARTAPI.RayTracing.EmbreeBVH BVH = new DHARTAPI.RayTracing.EmbreeBVH(Mesh);

            // Size of test
            const int arraysize = 1000000;

            Vector3D[] origin_vector = new Vector3D[arraysize];
            Vector3D[] direction_vector = new Vector3D[1];

            direction_vector[0] = new Vector3D(0, 0, -1);

            for (int i = 0; i < arraysize; i++)
            {
                origin_vector[i] = new Vector3D(0, 0, 1);
            }

            // Start the stopwatch test
            stopwatch.Start();

            bool[] results = DHARTAPI.RayTracing.EmbreeRaytracer.IntersectOccluded(BVH, origin_vector, direction_vector, -1);

            // End the stopwatch
            stopwatch.Stop();

            foreach (bool result in results)
                Assert.IsTrue(result);

            Console.WriteLine(" Passing Vector3D array took {0} milliseconds", stopwatch.Elapsed.TotalMilliseconds);
        }

        [TestMethod]
        public void Flat()
        {
            var stopwatch = new Stopwatch();

            MeshInfo Mesh = OBJLoader.LoadOBJ(plane_path, CommonRotations.Yup_To_Zup);
            DHARTAPI.RayTracing.EmbreeBVH BVH = new DHARTAPI.RayTracing.EmbreeBVH(Mesh);

            // Size of test
            const int arraysize = 1000000;

            float[] origin_vector = new float[arraysize * 3];
            float[] direction_vector = new float[3];

            direction_vector[0] = 0.0f;
            direction_vector[1] = 0.0f;
            direction_vector[2] = -1.0f;

            for (int i = 0; i < arraysize; i++)
            {
                int stride = i * 3;
                origin_vector[stride] = 0f;
                origin_vector[stride + 1] = 0f;
                origin_vector[stride + 2] = 1f;
            }

            // Start the stopwatch test
            stopwatch.Start();

            bool[] results = DHARTAPI.RayTracing.EmbreeRaytracer.IntersectOccluded(BVH, origin_vector, direction_vector, -1);

            // End the stopwatch
            stopwatch.Stop();

            foreach (bool result in results)
                Assert.IsTrue(result);

            Console.WriteLine(" Passing already Flat array took {0} milliseconds", stopwatch.Elapsed.TotalMilliseconds);
        }
    }
}