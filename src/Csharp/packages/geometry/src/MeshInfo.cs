using System;
using System.Security.Cryptography;

/*!
	\brief Manipulate and load geometry from disk.

	\details
	The Geometry namespace contains all functionality in HumanFactors for dealing with mesh Geometry. The fundamental
	datatype of HumanFactors.Geometry is MeshInfo, which stores the vertices and indices of a mesh in unmanaged memory.

	\remarks
	Right now only OBJ files are supported for loading geometry from disk, however MeshInfo offers
	multiple constructors that accept arrays of vertices and indices so support for more formats
	can be developed.

	\see OBJLoader to load OBJ files from disk.
	\see MeshInfo for details on how HumanFactors represents mesh geometry, and how to convert a mesh
    into a format that humanfactors can use. 
*/

namespace HumanFactors.Geometry
{
    /*!
		\brief A collection of vertices and indices representing geometry.

        \image html https://upload.wikimedia.org/wikipedia/commons/2/2d/Mesh_fv.jpg "Figure 1.1: Meshinfo Internals"

        \details
        Stores a reference to mesh geometry in native memory. Internally, meshes are represented as a 3 by X matrix
        of vertices and a 3 by X matrix for indices. In the above image, the face list and vertex list are what's held
        in MeshInfo's Index and Vertex arrays respectively.

	    \invariant Will always hold a valid mesh.

		\internal
            \remarks
            Eigen is used to manage all matricies and perform quick transformations such
            as RotateMesh. More details on Eigen are available here:
            https://eigen.tuxfamily.org/dox/group__Geometry__Module.html

            \todo Support getting the name of a mesh from the OBJ. 
            \todo Support transforming by a matrix. 
        \endinternal
    */

    public class MeshInfo : NativeUtils.NativeObject
	{
		/*!
            \brief Calculates the mesh's pressure. Unimplemented for now
            \todo
            Make a function in C++ to calculate the size of indices/vertices so
            the amount of pressure to exert on the GC can properly be calculated.
        */

		private static int CalculatePresure()
		{
			return -1; // We don't really know the pressure for this class
		}

		/*!
            \brief Manually delete this mesh in Unmanaged memory.

            \remarks
            This is called automatically when the class is garbage collected, so don't worry about needing
            to manually call this. It may be advantageous to deallocate large meshes at specific times for
            specialized purposes where the garbage collector deleting a large mesh will cause noticable
            lag.

            \returns
            True if the handle is released successfully; otherwise, in the event of a catastrophic
            failure, false. In this case, it generates a releaseHandleFailed MDA Managed Debugging Assistant.

            \warning
            This deletes the mesh in unmanaged memory! Don't try to use this class again after calling this function!
        */
		protected override bool ReleaseHandle()
		{
			NativeMethods.DestroyMeshInfo(this.Pointer);
			return true;
		}

        /*!
            \brief Create a new meshinfo instance from the given pointer. </summary>

            \param pointer Pointer to an existing meshinfo instance in unmanaged memory

            \details Just a call back to the base functionality of nativeobject. 

            \remarks This shouldn't be called directly unless pointer is gauranteed to point to a valid mesh
        */

        internal MeshInfo(IntPtr pointer, int size = 0) : base(pointer, size)
        {
        }

        /*!
            \brief Create an instance of MeshInfo from an array of vertices and triangle indices.

            \param indices
            An array of indices for the triangles in the mesh. Each integer should correspond to 3
            values in <paramref name="vertices" />, and every 3 integers should represent a complete
            triangle for the mesh.
            \param vertices Vertices of the mesh. Each 3 floats represent the X,Y, and Z of a point in space
            \param name The name of the mesh. Unused for now.
            \param id The unique identifier for this mesh. If -1, this will automatically be set

            \throws HumanFactors.Exceptions.InvalidMeshException
            The input indices and vertices result in an invalid mesh.
        */

        public MeshInfo(int[] indices, float[] vertices, string name = "", int id = -1) :
			base(NativeMethods.C_StoreMesh(vertices, indices, name, id), (vertices.Length * 4) + (indices.Length * 4))
		{ }

		/*!
            \brief Rotate this mesh by the desired magnitude.

            \param xrot Pitch to rotate by in degrees.
		    \param yrot Yaw to rotate by in degrees.
	        \param zrot Roll to rotate by in degrees.

            \remarks
            See the other overload for this function for use with CommonRotations.
        */

		public void RotateMesh(float xrot, float yrot, float zrot) => NativeMethods.C_RotateMesh(handle, xrot, yrot, zrot);

		/*!
            \brief Rotate this mesh by the desired magnitude. </summary>
            \param rotation How far to rotate the mesh on the X,Y, and Z, axises in degrees.

            \see CommonRotations for commonly used rotations such as Yup to Zup.
        */

		public void RotateMesh(Vector3D rotation) => NativeMethods.C_RotateMesh(handle, rotation.x, rotation.y, rotation.z);
	}
}