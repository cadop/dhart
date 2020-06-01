using System;
using System.Security.Cryptography;

namespace HumanFactors.Geometry
{
    /// <summary> Wrapper for a mesh in unmanaged memory. </summary>
    public class MeshInfo : NativeUtils.NativeObject
    {
        /// <summary> Calculates the mesh's pressure. Unimplemented for now </summary>
        private static int CalculatePresure()
        {
            return -1; // We don't really know the pressure for this class
        }

        /// <summary> Executes the code required to delete the mesh </summary>
        /// <returns>
        /// true if the handle is released successfully; otherwise, in the event of a catastrophic
        /// failure, false. In this case, it generates a releaseHandleFailed MDA Managed Debugging Assistant.
        /// </returns>
        protected override bool ReleaseHandle()
        {
            NativeMethods.DestroyMeshInfo(this.Pointer);
            return true;
        }

        /// <summary> Create a new meshinfo instance from the given pointer. </summary>
        /// <param name="pointer"> Pointer to an existing meshinfo instance in unmanaged memory. </param>
        /// <remarks>
        /// This shouldn't be called directly unless pointer is gauranteed to point to a valid mesh
        /// in unmanaged memory.
        /// </remarks>
        public MeshInfo(IntPtr pointer, int size = 0) : base(pointer, size) { }

        /// <summary>
        /// Create a new instance of <see cref="MeshInfo" /> from an array of vertices and triangle indices.
        /// </summary>
        /// <param name="indices">
        /// An array of indices for the triangles in the mesh. Each integer should correspond to 3
        /// values in <paramref name="vertices" />, and every 3 integers should represent a complete
        /// triangle for the mesh.
        /// </param>
        /// <param name="vertices">
        /// Vertices of the mesh. Each 3 floats represent the X,Y, and Z of a point in space.
        /// </param>
        /// <param name="name"> The name of the mesh. Unused for now. </param>
        /// <param name="id">
        /// The unique identifier for this mesh. If -1, this will automatically be set
        /// </param>
        /// <exception cref="HumanFactors.Exceptions.InvalidMeshException">
        /// The input <paramref name="indices" /> and <paramref name="vertices" /> result in an
        /// invalid mesh.
        /// </exception>
        public MeshInfo(int[] indices, float[] vertices, string name = "", int id = -1) :
            base(NativeMethods.C_StoreMesh(vertices, indices, name, id), (vertices.Length * 4) + (indices.Length * 4))
        { }

        /// <summary> Rotates this mesh by the desired magnitude. </summary>
        /// <param name="xrot"> Degrees to rotate the mesh on the z axis. </param>
        /// <param name="yrot"> Degrees to rotate the mesh on the y axis. </param>
        /// <param name="zrot"> Degrees to rotate the mesh on the z axis. </param>
        public void RotateMesh(float xrot, float yrot, float zrot) => NativeMethods.C_RotateMesh(handle, xrot, yrot, zrot);

        /// <summary> Rotate this mesh by the desired magnitude. </summary>
        /// <param name="rotation"> How far to rotate the mesh on the X,Y, and Z, axises in degrees. </param>
        public void RotateMesh(Vector3D rotation) => NativeMethods.C_RotateMesh(handle, rotation.x, rotation.y, rotation.z);
    }
}