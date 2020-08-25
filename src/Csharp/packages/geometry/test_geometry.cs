using HumanFactors.Geometry;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace HumanFactors.Tests.Geometry
{
    [TestClass]
    public class OBJLoading
    {
        private TestContext testContextInstance;
        float[] global_vertices = {
            -20.079360961914062f, 0.0f, 18.940643310546875f,
            -20.079360961914062f, 0.0f, -18.842348098754883f,
            20.140586853027344f, 0.0f, 18.940643310546875f,
            20.140586853027344f, 0.0f, -18.842348098754883f 
        };

        int[] global_indices = { 
            3, 1, 0,
            2, 3, 0
        };

        /// <summary>
        ///  Gets or sets the test context which provides
        ///  information about and functionality for the current test run.
        ///</summary>
        public TestContext TestContext
        {
            get { return testContextInstance; }
            set { testContextInstance = value; }
        }

        private const string good_mesh_path = "ExampleModels/plane.obj";
        private const string missing_mesh_path = "bad_mesh";
        private const string non_mesh_path = "HumanFactors.dll";

        [TestMethod]
        public void SuccessLoadingOBJ()
        {
            HumanFactors.Geometry.MeshInfo MI = HumanFactors.Geometry.OBJLoader.LoadOBJ(good_mesh_path);
            Assert.IsTrue(true);
        }

        [TestMethod]
        [ExpectedException(typeof(System.IO.FileNotFoundException))]
        public void ThrowsNotFound()
        {
            HumanFactors.Geometry.MeshInfo MI = HumanFactors.Geometry.OBJLoader.LoadOBJ(missing_mesh_path);
        }

        [TestMethod]
        [ExpectedException(typeof(HumanFactors.Exceptions.InvalidMeshException))]
        public void ThrowsInvalid()
        {
            HumanFactors.Geometry.MeshInfo MI = HumanFactors.Geometry.OBJLoader.LoadOBJ(non_mesh_path);
            Assert.IsTrue(true);
        }

        [TestMethod]
        public void DirectMeshTransfer()
        {
            //! [EX_MeshInfoCstor]
            // These vertices/indices are directly taken from the plane OBJ in example models.
            // Every 3 floats represent the x,y,z locations for a seperate vertex. It takes only
            // 4 vertices to make a plane. 
            float[] vertices = { 
                -20.079360961914062f, 0.0f, 18.940643310546875f,
                -20.079360961914062f, 0.0f, -18.842348098754883f,
                20.140586853027344f, 0.0f, 18.940643310546875f, 
                20.140586853027344f, 0.0f, -18.842348098754883f 
            };

            // Every 3 ints represent the indexes of vertices for a seperate triangle. 
            int[] indices = { 
                3, 1, 0, 
                2, 3, 0 
            };

            // Construct a new meshinfo instance with these elements
            HumanFactors.Geometry.MeshInfo Mesh = new HumanFactors.Geometry.MeshInfo(indices, vertices);
            //! [EX_MeshInfoCstor]
        }
        [TestMethod]
        public void MeshRotate()
        {
            float[] vertices = { -20.079360961914062f, 0.0f, 18.940643310546875f, -20.079360961914062f, 0.0f, -18.842348098754883f, 20.140586853027344f, 0.0f, 18.940643310546875f, 20.140586853027344f, 0.0f, -18.842348098754883f };
            int[] indices = { 3, 1, 0, 2, 3, 0 };

            HumanFactors.Geometry.MeshInfo Mesh = new HumanFactors.Geometry.MeshInfo(indices, vertices);

            //! [EX_RotateMesh_Common]

            // Rotate the mesh using a common rotation to convert it from yup to zup
            Mesh.RotateMesh(CommonRotations.Yup_To_Zup);

            //! [EX_RotateMesh_Common]

            //! [EX_RotateMesh_xyz]

            // Rotate the mesh 90 degrees around the X axis
            Mesh.RotateMesh(90, 0, 0);

            //! [EX_RotateMesh_xyz]
        }
        
        [TestMethod]
        public void NumberOfSubmeshesLoadedIsCorrect()
        {
            //! [EX_LoadSubmeshes]
            
            // Load submeshes
            MeshInfo[] submeshes = OBJLoader.LoadOBJSubmeshes("ExampleModels/sponza.obj", GROUP_METHOD.BY_GROUP);

            // Print the first five
            for(int i = 0; i < 5; i++)
                Debug.WriteLine(submeshes[i]);

            //! [EX_LoadSubmeshes]
            Assert.AreEqual(33, submeshes.Length, "Number of submeshes didn't match the actual number of submeshes in sponza");
        }

        [TestMethod]
        public void NameAndIDAreSet()
        {
            string test_name = "MeshName";
            int test_id = 39;
            MeshInfo MI = new MeshInfo(global_indices, global_vertices, test_name, test_id);

            Assert.AreEqual(test_name, MI.name);
            Assert.AreEqual(test_id, MI.id);
        }

        [TestMethod]
        public void VertsAndTrisMatchInput()
        {
            // Construct mesh
            MeshInfo Mesh = new MeshInfo(global_indices, global_vertices);



            // get verts and indices as arrays. 
            var flat_verts = Mesh.vertices.array;
            var flat_indices = Mesh.indices.array;

            // Print to console
            Debug.WriteLine(Mesh.vertices);
            Debug.WriteLine(Mesh.indices);

            // Assert that the returned length is equal
            Assert.AreEqual(global_vertices.Length, flat_verts.Length);
            Assert.AreEqual(global_indices.Length, flat_indices.Length);

            // Assert that both arrays have the same contents as their inputs
            for (int i = 0; i < flat_verts.Length; i++)
                Assert.AreEqual(flat_verts[i], global_vertices[i]);
            for (int i = 0; i < flat_indices.Length; i++)
                Assert.AreEqual(flat_indices[i], global_indices[i]);
        }
    }
}