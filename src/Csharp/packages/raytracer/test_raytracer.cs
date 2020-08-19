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

            //! [EX_BVH_CSTOR]

            // Load an OBJ from a filepath  into a new meshinfo instance
            string good_mesh_path = "ExampleModels/plane.obj";
            MeshInfo Mesh = OBJLoader.LoadOBJ(good_mesh_path, CommonRotations.Yup_To_Zup);

            // Construct a BVH from the mesh
            EmbreeBVH BVH = new EmbreeBVH(Mesh);

            //! [EX_BVH_CSTOR]
        }

        [TestMethod]
        public void CreateWithMultipleMeshes(){
            
            // Load a lot of submeshes from sponza
            var MeshInfos = OBJLoader.LoadOBJSubmeshes("ExampleModels/sponza.obj", GROUP_METHOD.BY_GROUP);
            
            // Create a BVH for these meshes. If this crashes, then there's an issue we care about
            EmbreeBVH bvh = new EmbreeBVH(MeshInfos);

            // Cast a ray straight downwards, and ensure it intersects.
            // If the point is false, then the bvh was created incorrectly
            Vector3D origin = new Vector3D(0, 0, 1);
            Vector3D direction = new Vector3D(0, 0, -1);
            var pt = EmbreeRaytracer.IntersectForPoint(bvh, origin, direction);

            Assert.IsTrue(pt.IsValid());
        }

    }

    [TestClass]
    public class TestRayTracer
    {
        private const string plane_path = "ExampleModels/plane.obj";

        public EmbreeBVH LoadPlane()
        {
            //! [LoadPlane]

            // Load a plane into a meshinfo instance from disk
            string plane_path = "ExampleModels/plane.obj";
            MeshInfo Mesh = OBJLoader.LoadOBJ(plane_path, CommonRotations.Yup_To_Zup);

            // Construct a BVH from it
            EmbreeBVH BVH = new EmbreeBVH(Mesh);

            //! [LoadPlane]

            return BVH;
        }

        public bool AreNear(float f1, float f2) => Math.Abs(f1 - f2) < 0.001f;

        [TestMethod]
        public void IntersectForPoint()
        {
            EmbreeBVH BVH = LoadPlane();

            //! [EX_IntForPointSingle]

            // Define an origin and direction
            Vector3D origin = new Vector3D(0, 0, 1);
            Vector3D direction = new Vector3D(0, 0, -1);
            
            // Cast a ray from origin an direction and get the point
            Vector3D good_point = HumanFactors.RayTracing.EmbreeRaytracer.IntersectForPoint(BVH, origin, direction, -1);

            // Print the point
            Debug.WriteLine(good_point);
            
            //! [EX_IntForPointSingle]

            //! [EX_IntForPointSingle_2]
           
            // Do the same in a direction we know will miss
            Vector3D bad_direction = new Vector3D(0, 0, 1);
            Vector3D bad_point = HumanFactors.RayTracing.EmbreeRaytracer.IntersectForPoint(BVH, origin, bad_direction, -1);
           
            // Print the result of the missed ray
            Debug.WriteLine(bad_point);
            
            //! [EX_IntForPointSingle_2]


            Assert.IsTrue(good_point.IsValid());
            Assert.IsTrue(!bad_point.IsValid());
        }

        [TestMethod]
        public void IntersectForPoint_xyz()
        {
            EmbreeBVH BVH = LoadPlane();

            //! [EX_IntForPointSinglexyz]

            // Define the X,Y,Z coordinates for the origin and direction
            float x = 0; float y = 0; float z = 1;
            float dx = 0; float dy = 0; float dz = -1;           

            // Cast a ray from origin an direction and get the point
            Vector3D good_point = HumanFactors.RayTracing.EmbreeRaytracer.IntersectForPoint(
                BVH, x, y, z, dx, dy, dz, -1
           );

            // Print the point
            Debug.WriteLine(good_point);

            //! [EX_IntForPointSinglexyz]

            //! [EX_IntForPointSinglexyz_2]

            // Do the same in a direction we know will miss
            float bad_dir_x = 0; float bad_dir_y = 0; float bad_dir_z = 1;
            Vector3D bad_point = HumanFactors.RayTracing.EmbreeRaytracer.IntersectForPoint(
                BVH, x, y, z, bad_dir_x, bad_dir_y, bad_dir_z, -1
           );
            // Print the result of the missed ray
            Debug.WriteLine(bad_point);

            //! [EX_IntForPointSinglexyz_2]


            Assert.IsTrue(good_point.IsValid());
            Assert.IsTrue(!bad_point.IsValid());
        }

        [TestMethod]
        public void PointsEqualDirAndOrigin()
        {
            EmbreeBVH BVH = LoadPlane();


            //! [EX_FireRayMultiplePoints]

            int num_points = 5;

            // Create and fill a vector of origins and directions
            Vector3D[] origin_vector = new Vector3D[num_points];
            Vector3D[] direction_vector = new Vector3D[num_points];
            for (int i = 0; i < num_points; i++)
            {
                origin_vector[i] = new Vector3D(0, i, 1);
                direction_vector[i] = new Vector3D(0, 0, -1);
            }

            // Cast all rays in a single call
            Vector3D[] results = EmbreeRaytracer.IntersectForPoints(BVH, origin_vector, direction_vector, -1);

            // Print results
            for (int i = 0; i < num_points; i++)
                Debug.WriteLine(String.Format("Origin: {0}, Intersection: {1}", origin_vector[i], results[i]));

            //! [EX_FireRayMultiplePoints]

            var result_1 = results[0];
            for (int i = 0; i < num_points; i++)
            {
                Vector3D expected_hit = new Vector3D(0, origin_vector[i].y, 0);
                Vector3D actual_hit = results[i];

                Assert.IsTrue(0.001f > actual_hit.DistanceTo(expected_hit));
            }
        }


        [TestMethod]
        public void DistanceSingleRay()
        {
            EmbreeBVH BVH = LoadPlane();

            //! [EX_IntersectForDistance]
           
            // Create origin and direction to cast ray straight down at the plane
            Vector3D origin = new Vector3D(0, 0, 1);
            Vector3D direction = new Vector3D(0, 0, -1);

            // Cast ray and get the distance/MeshID
            RayResult result = EmbreeRaytracer.IntersectForDistance(BVH, origin, direction);
            

            // Print the distance/MeshID
            Debug.WriteLine(result);

            //! [EX_IntersectForDistance]
            Assert.IsTrue(Math.Abs(result.distance - 1.0f) < 0.001f);
        }

        [TestMethod]
        public void DistanceSingleRay_MultipleTrials()
        {
            EmbreeBVH BVH = LoadPlane();

            // C# has no function/constructor to just fill an array with a value?
            RayResult[] results = new RayResult[100];
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
                Assert.AreEqual(0, result_1.DistanceTo(result), 0.0001, "Distance was greater than expected");
        }

        [TestMethod]
        public void MultipleRaysDistance()
        {
            EmbreeBVH BVH = LoadPlane();

            //! [EX_IntersectForDistances]

            // Construct origin/distance arrays
            int num_rays = 6;
            Vector3D[] origin_vector = new Vector3D[num_rays];
            Vector3D[] direction_vector = new Vector3D[num_rays];
            for (int i = 0; i < num_rays; i++)
            {
                origin_vector[i] = new Vector3D(0, 0, i);
                direction_vector[i] = new Vector3D(0, 0, -1);
            }
            
            // Cast rays and get results
            RayResults results = EmbreeRaytracer.IntersectForDistances(BVH, origin_vector, direction_vector);
                
            // Print Results. The distance should equal the origin's z value, except for the first one, which
            // should miss since it's inside the plane
            for(int i = 0; i < num_rays; i++)
                Debug.WriteLine("Origin: {0}, Result: {1}", origin_vector[i], results[i]);

            //! [EX_IntersectForDistances]

            // Check results
            for (int i = 1; i < num_rays; i++)
                Assert.IsTrue(AreNear(origin_vector[i].z, results[i].distance));
            
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

        [TestMethod]
        public void OcclusionRay()
        {
            EmbreeBVH BVH = LoadPlane();

            //! [EX_Occlusion]

            // Create origin and direction arrays
            Vector3D[] origins = { new Vector3D(0, 0, 1), new Vector3D(0, 0, -1) };
            Vector3D[] directions = { new Vector3D(0, 0, -1), new Vector3D(0, 0, -1) };

            // Cast a ray from the origin and direction at every matching index 
            bool[] intersections = EmbreeRaytracer.IntersectOccluded(BVH, origins, directions, -1);

            // Print the results. Ray 1 should intersect the plane, and ray 2 should miss it. 
            Debug.WriteLine(String.Format("Ray 1: {0}, Ray 2 : {1}", intersections[0], intersections[1]));

            //! [EX_Occlusion]

            Assert.IsTrue(intersections[0]);
            Assert.IsFalse(intersections[1]);
        }
    }
}