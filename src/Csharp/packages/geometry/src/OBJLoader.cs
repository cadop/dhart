using System;

namespace HumanFactors.Geometry
{
    /// <summary>
    /// Contains functions to load a mesh from an obj file.
    /// </summary>
    public static class OBJLoader
    {
        /// <summary>
        /// Attempt to load the OBJ at the given filepath. If a rotation is specifed, the mesh will be rotated
        /// </summary>
        /// <param name="path"> Filepath for the obj to load. </param>
        /// <param name="xrot"> Degrees to rotate the mesh on the z axis. </param>
        /// <param name="yrot"> Degrees to rotate the mesh on the y axis. </param>
        /// <param name="zrot"> Degrees to rotate the mesh on the z axis. </param>
        /// <returns>
        /// A new instance of <see cref="MeshInfo" /> holding a reference to the unmanaged mesh.
        /// </returns>
        /// <exception cref="System.IO.FileNotFoundException">
        /// No file was found at the given path.
        /// </exception>
        /// <exception cref="HumanFactors.Exceptions.InvalidMeshException">
        /// The file at the given path did not represent a valid OBJ.
        /// </exception>
        /// <example>
        /// Loading a mesh from plane.obj with no rotation.
        /// <code> MeshInfo MI = OBJLoader.LoadOBJ("plane.obj"); </code>
        /// </example>
        public static MeshInfo LoadOBJ(string path, float xrot = 0, float yrot = 0, float zrot = 0)
        {
            var mesh_ptr = NativeMethods.C_LoadOBJ(path, xrot, yrot, zrot);
            return new MeshInfo(mesh_ptr);
        }

        /// <summary>
        /// Attempt to load the OBJ at the given filepath. If a rotation is specifed, the mesh will be rotated.
        /// </summary>
        /// <param name="path"> Filepath for the obj to load </param>
        /// <param name="rotation"> X,Y, and Z values in degrees to rotate the mesh </param>
        /// <returns>
        /// A new instance of <see cref="MeshInfo" /> holding a reference to the unmanaged mesh.
        /// </returns>
        /// <exception cref="System.IO.FileNotFoundException">
        /// No file was found at the given path
        /// </exception>
        /// <exception cref="HumanFactors.Exceptions.InvalidMeshException">
        /// The file at the given path was not a valid OBJ
        /// </exception>
        /// <example>
        /// Using a predefined rotation from  <see cref="CommonRotations"/> to load a mesh then convert it from Y-up to Z-up
        /// <code>MeshInfo MI = OBJLoader.LoadOBJ("plane.obj", CommonRotations.Yup_To_Zup)</code>
        /// </example>
        ///<remarks>This overload is useful in combination with <see cref="CommonRotations"/>. </remarks>
        public static MeshInfo LoadOBJ(string path, Vector3D rotation) => LoadOBJ(path, rotation.x, rotation.y, rotation.z);
    }
}