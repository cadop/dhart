using Microsoft.VisualStudio.TestTools.UnitTesting;
using HumanFactors.Geometry;
using HumanFactors.RayTracing;
using System.Runtime.InteropServices;
using System;
using System.IO;
using HumanFactors.ViewAnalysis;
using System.Diagnostics;
using HumanFactors.NativeUtils.CommonNativeArrays;

namespace HumanFactors.Tests.ViewAnalysis
{

    [TestClass]
    public class ViewAnalysis
    {
        private EmbreeBVH GetBVH()
        {
            //! [EX_GetBVH]
            // Load the plane .obj from example models, and rotate it from y-up to z-up
            string mesh_path = "ExampleModels/plane.obj";
            MeshInfo OBJ = OBJLoader.LoadOBJ(mesh_path, CommonRotations.Zup_To_Yup);

            // Create a BVH from the plane
            EmbreeBVH bvh = new EmbreeBVH(OBJ);

            Vector3D Point = new Vector3D(0, 0, 0);
            Vector3D[] nodes = { Point };
            int num_rays = 1000;

            //! [EX_GetBVH]
            return bvh;
        }
        
        [TestMethod]
        public void AggregateSucceed()
        {
            var bvh = GetBVH();

            //! [EX_ViewAnalysisAggregate]

            // Define the nodes to evaluate
			Vector3D[] nodes = {
				new Vector3D(0,0,1),
				new Vector3D(0,0,2),
				new Vector3D(0,0,3)
			};

            // Set maximum number of rays, then conduct analysis
	        int num_rays = 1000;
			ManagedFloatArray results = HumanFactors.ViewAnalysis.ViewAnalysis.ViewAnalysisAggregate(
				bvh,
				nodes,
				num_rays,
				type:ViewAggregateType.COUNT
		    );

            // Print results. The number of rays should get lower with each element, since each node
            // is further away from the plane than the last
            Debug.WriteLine(results);

            //! [EX_ViewAnalysisAggregate]

            for (int i = 1; i < nodes.Length; i++)
                Assert.IsTrue(results[i] < results[i - 1]);
        }   

        [TestMethod]
        public void NonAggregateSucceed()
        {
            var bvh = GetBVH();

            //! [EX_ViewAnalysisStandard]
            // Define the nodes to evaluate
            Vector3D[] nodes = {
                new Vector3D(0,0,1),
                new Vector3D(1,1,1),
                new Vector3D(0,0,3)
            };

            // Set maximum number of rays, then conduct analysis
            int num_rays = 10;

            // Conduct view analysis and get all the results
            var results = HumanFactors.ViewAnalysis.ViewAnalysis.ViewAnalysisStandard(bvh, nodes, num_rays);

            // Iterate through every node and print the results
            for (int node_index = 0; node_index < nodes.Length; node_index++)
            {
                var node = nodes[node_index];
                string print = node.ToString() + ": (";
                for (int ray_index = 0; ray_index < num_rays; ray_index++)
                {
                    // Get the result for the current node at the current ray
                    var result = results[node_index][ray_index];
                    print += result.ToString();

                    // Add a comma if it isn't the last element
                    if (ray_index != num_rays - 1)
                        print += ", ";
                    else
                        print += ")";
                }
                // Print the line
                Debug.WriteLine(print);
                    
            }

            //! [EX_ViewAnalysisStandard]
            var arr = results.array;

            Assert.IsTrue(arr.Length > 0);
            Assert.IsTrue(results[0].Length > 0);
        }
        [TestMethod]
        public void SphereDistributeRays()
        {
            var bvh = GetBVH();

            //! [EX_SphericallyDistributeRays]

            // Equally distribute 10 rays
            var results = HumanFactors.ViewAnalysis.ViewAnalysis.SphericallyDistributeRays(10);

            // Print results
            Debug.WriteLine(results);

            //! [EX_SphericallyDistributeRays]
            var direction = results[0];
            Assert.AreEqual(direction.Length, 3);
        }
    }
}