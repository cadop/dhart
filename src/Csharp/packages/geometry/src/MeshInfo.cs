using DHARTAPI.NativeUtils;
using System;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Security.Cryptography;

/*!
	\brief Manipulate and load geometry from disk.

	\details
	The Geometry namespace contains all functionality in DHARTAPI for dealing with mesh Geometry. The fundamental
	datatype of DHARTAPI.Geometry is MeshInfo, which stores the vertices and indices of a mesh in unmanaged memory.

	\remarks
	Right now only OBJ files are supported for loading geometry from disk, however MeshInfo offers
	multiple constructors that accept arrays of vertices and indices so support for more formats
	can be developed.

	\see OBJLoader to load OBJ files from disk.
	\see MeshInfo for details on how DHARTAPI represents mesh geometry, and how to convert a mesh
    into a format that DHART_API can use. 
*/

namespace DHARTAPI.Geometry
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
        public int id = -1; ///< ID of the mesh. 
        public string name = ""; ///< Name of the mesh
        public DependentNativeArray<float> vertices; ///< An array of all the coordinates for every vertex in the mesh
        public DependentNativeArray<int> indices; ///< An array of the indices for every triangle in the mesh

        /*!
            \brief Calculates the amount of pressure this mesh should exert on the GC.

            \returns The approximate size of this mesh in bytes.
        */
        public int CalculatePresure()
		{
            int num_verts = this.vertices.size;
            int num_tris = this.indices.size;

            int float_pressure = (sizeof(float) * num_verts * 3);
            int int_pressure = (sizeof(float) * num_tris * 3);

            return int_pressure + float_pressure;
		}

        /*! \brief Updates this mesh's name, ID, and arrays with it's values from C++ */
        internal void UpdateIDNameAndArrays()
        {
            // Get ID and name
            this.id = NativeMethods.GetMeshID(this.Pointer);
            this.name = NativeMethods.GetMeshName(this.Pointer);

            // Pointers and size of verts and tris arrays
            var verts_and_tris = NativeMethods.C_GetTrisAndVerts(this.Pointer);

            // Store these locally
            this.vertices = new DependentNativeArray<float>(verts_and_tris.vert_ptr, verts_and_tris.verts, 3);
            this.indices = new DependentNativeArray<int>(verts_and_tris.tri_ptr, verts_and_tris.tris, 3);
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
            \brief Create a new meshinfo instance from the given pointer.

            \param pointer Pointer to an existing meshinfo instance in unmanaged memory.
            \param size The size of this meshinfo in bytes. Leave at 0 to use default behavior.
            
            \details Just a call back to the base functionality of nativeobject. 

            \remarks This shouldn't be called directly unless pointer is gauranteed to point to a valid mesh
        */

		internal MeshInfo(IntPtr pointer, int size = 0) : base(pointer, size) {
            UpdateIDNameAndArrays();
            this.UpdatePressure(this.CalculatePresure());
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

            \throws DHARTAPI.Exceptions.InvalidMeshException
            The input indices and vertices result in an invalid mesh.

            \par Example
            \snippet geometry\test_geometry.cs EX_MeshInfoCstor
        */

		public MeshInfo(int[] indices, float[] vertices, string name = "", int id = 0) :
			base(NativeMethods.C_StoreMesh(vertices, indices, name, id))
		{ 
            UpdateIDNameAndArrays();
            this.UpdatePressure(this.CalculatePresure());
        }

        /*!
            \brief Rotate this mesh by the desired magnitude.

            \param xrot Pitch to rotate by in degrees.
		    \param yrot Yaw to rotate by in degrees.
	        \param zrot Roll to rotate by in degrees.

            \remarks
            See the other overload for this function for use with CommonRotations.

            \par Example
            \snippet geometry\test_geometry.cs EX_MeshInfoCstor
            \snippet geometry\test_geometry.cs EX_RotateMesh_xyz
        */

        public void RotateMesh(float xrot, float yrot, float zrot)
        {
            NativeMethods.C_RotateMesh(handle, xrot, yrot, zrot);
            UpdateIDNameAndArrays();
        }

		/*!
            \brief Rotate this mesh by the desired magnitude. </summary>
            \param rotation How far to rotate the mesh on the X,Y, and Z, axises in degrees.

            \see CommonRotations for commonly used rotations such as Yup to Zup.

            \par Example
            \snippet geometry\test_geometry.cs EX_MeshInfoCstor
            \snippet geometry\test_geometry.cs EX_RotateMesh_Common
        */
		public void RotateMesh(Vector3D rotation) => NativeMethods.C_RotateMesh(handle, rotation.x, rotation.y, rotation.z);

        public override String ToString()
        {
            return "Name: " + this.name + ", ID: " + id.ToString() + ", Verts: " + vertices.size.ToString() + ", Triangles: " + (this.indices.size.ToString());
        }
    }
}