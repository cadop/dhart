using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using System.Diagnostics;

namespace HumanFactors.Tests.Geometry
{
    [TestClass]
    public class OBJLoading
    {
        private TestContext testContextInstance;

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
            // This is taken directly from the plane OBJ
            float[] vertices = { -20.079360961914062f, 0.0f, 18.940643310546875f, -20.079360961914062f, 0.0f, -18.842348098754883f, 20.140586853027344f, 0.0f, 18.940643310546875f, 20.140586853027344f, 0.0f, -18.842348098754883f };
            int[] indices = { 3, 1, 0, 2, 3, 0 };

            HumanFactors.Geometry.MeshInfo Mesh = new HumanFactors.Geometry.MeshInfo(indices, vertices);
        }
        [TestMethod]
        public void MeshRotate()
        {
            // This is taken directly from the plane OBJ
            float[] vertices = { -20.079360961914062f, 0.0f, 18.940643310546875f, -20.079360961914062f, 0.0f, -18.842348098754883f, 20.140586853027344f, 0.0f, 18.940643310546875f, 20.140586853027344f, 0.0f, -18.842348098754883f };
            int[] indices = { 3, 1, 0, 2, 3, 0 };

            HumanFactors.Geometry.MeshInfo Mesh = new HumanFactors.Geometry.MeshInfo(indices, vertices);
            Mesh.RotateMesh(90, 0, 0);
        }
    }
}