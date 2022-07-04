using DHARTAPI.NativeUtils;
using System;
using System.Linq.Expressions;
using System.Runtime.InteropServices;
using DHARTAPI.Exceptions;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.CodeDom;
using System.ComponentModel;
using System.Diagnostics;

namespace DHARTAPI.Geometry
{

	/*! \brief Native methods for the Geometry namespace */
	internal static class NativeMethods
	{
		internal struct TrisAndVertsReturn {
			public IntPtr tri_ptr;
			public IntPtr vert_ptr;
			public int tris;
			public int verts;
		}


		/// <summary>
		/// Path to the DHARTAPI dll copied from <see cref="NativeConstants"/>.
		/// </summary>
		const string dllpath = NativeConstants.DLLPath;

		/*!
            \brief Load an obj file from the given filepath into a MeshInfo object in native memory.
            
            \param obj_path Path to tthe obj file to load
            \param xrot Degrees to rotate the mesh on the x axis.
            \param yrot Degrees to rotate the mesh on the y axis.
            \param zrot Degrees to rotate the mesh on the z axis.
            
            \returns A pointer to the a instance of MeshInfo in native memory containing the mesh at `path`.

            \throws System.IO.FileNotFoundException No file was found at `path`.
			\throws DHARTAPI.Exceptions.InvalidMeshException The file at `path` did not represent a valid OBJ.
       */
		internal static IntPtr[] C_LoadOBJ(string obj_path, float xrot, float yrot, float zrot, GROUP_METHOD gm)
		{
			// Create a pointer as an output parameter
			IntPtr out_ptr = new IntPtr();

			// Call the function in C++ and capture the error code
			int num_meshes = 0;
			HF_STATUS res = LoadOBJ(obj_path, (int)gm, xrot, yrot, zrot, ref out_ptr, ref num_meshes);

			// If the file was not found, throw the standard file not found exception
			if (res == HF_STATUS.NOT_FOUND)
				throw new FileNotFoundException(obj_path + " did not lead to any file");

			// If the file at the filepath didn't represent a valid OBJ model, throw InvalidMesh Exception
			else if (res == HF_STATUS.INVALID_MESH)
				throw new InvalidMeshException(obj_path + " did not lead to an obj file, or the obj file was invalid!");

			// Iterate through the returned pointer array, and marshall each of it's pointers
			// into managed memory
			IntPtr[] out_ptrs = new IntPtr[num_meshes];
			//for (int i = 0; i < num_meshes; i++)
			//	out_ptrs[i] = Marshal.ReadIntPtr(out_ptr, i * sizeof(IntPtr));

			Marshal.Copy(out_ptr, out_ptrs, 0, num_meshes);

			// Free the memory allocated by C++ for the pointer array
			DestroyMeshInfoPtrArray(out_ptr);	
		
			// Return managed pointer array
			return out_ptrs;
		}

		/*! \brief Get the ID of an instance of MeshInfo in c++.
			\param mesh_ptr Mesh to get the ID of
			\returns the ID of the mesh in native memory
		*/
		internal static int GetMeshID(IntPtr mesh_ptr)
		{
			// Create an output parameter then call the native function
			int out_int = -1;
			GetMeshID(mesh_ptr, ref out_int);

			// Assert it's different then return
			Debug.Assert(out_int != -1, "out_int didn't get updated");
			return out_int;
		}


		/*! \brief Get the name of an instance of MeshInfo in c++.
			\param mesh_ptr Mesh to get the Name of
			\returns the Name of the mesh in native memory
		*/
		internal static string GetMeshName(IntPtr mesh_ptr)
		{
			// Create an output parameter then call the native function
			IntPtr out_string = new IntPtr();
			GetMeshName(mesh_ptr, ref out_string);

			// Copy the string into managed memory. If it's corrupted in some way
			// then that will be reflected in a crash here
			String ManagedString = Marshal.PtrToStringAnsi(out_string);

			// Free the char array allocated by C++ now that we have the string
			DestroyCharArray(out_string);

			return ManagedString;
		}

		/*!
            \brief Construct an instance of meshinfo in native memory.

            \param indices
            An array of indices for the triangles in the mesh. Each integer should correspond to 3
            values in <paramref name="vertices" />, and every 3 integers should represent a complete
            triangle for the mesh.
            \param vertices Vertices of the mesh. Each 3 floats represent the X,Y, and Z of a point in space
            \param name The name of the mesh. Unused for now.
            \param id The unique identifier for this mesh. If -1, this will automatically be set
            
            \returns 
            A pointer to a new MeshInfo object in unmanaged memory, constructed with `vertices
            and `indices`.

            \throws DHARTAPI.Exceptions.InvalidMeshException The input indices and vertices result in an invalid mesh.
        */
		internal static IntPtr C_StoreMesh(IEnumerable<float> vertices, IEnumerable<int> indices, string name, int id)
		{

			// Convert IEnumerables to arrays
			var vert_array = vertices.ToArray<float>();
			var ind_array = indices.ToArray<int>();

			// Get the size of both arrays
			int num_verts = vertices.Count();
			int num_inds = indices.Count();

			// Create a pointer to use as a reference parameter
			IntPtr out_ptr = new IntPtr();

			// Call the native funciton 
			HF_STATUS res = StoreMesh(ref out_ptr, ind_array, num_inds, vert_array, num_verts, name, id);

			// If the error code indicates this mesh is invalid, throw an exception
			if (res == HF_STATUS.INVALID_MESH)
				throw new InvalidMeshException("That did not represent a valid mesh");

			return out_ptr;
		}

		/*! \brief Get pointers to the vertex and index arrays of a mesh in C++
			\param MI the mesh to get the vertex and index arrays of.
			\returns POinters and sizes of the index and verted arrays of `MI`.
		*/
		internal static TrisAndVertsReturn C_GetTrisAndVerts(IntPtr MI)
		{
			TrisAndVertsReturn ret = new TrisAndVertsReturn();

			GetVertsAndTris(MI, ref ret.tri_ptr, ref ret.tris, ref ret.vert_ptr, ref ret.verts);

			return ret;
		}

		/*! 
            \brief Rotate an instance of MeshInfo
           
            \param mesh Pointer to a MeshInfo object in native memory.
            \param xrot Degrees to rotate `mesh` on the x axis
            \param yrot Degrees to rotate `mesh` on the y axis
            \param zrot Degrees to rotate `mesh` on the z axis

            \post The mesh pointed to by `mesh` will be rotated as specified by `xrot`, `yrot` and `zrot`.
        */
		internal static void C_RotateMesh(IntPtr mesh, float xrot, float yrot, float zrot) => RotateMesh(mesh, xrot, yrot, zrot);

		[DllImport(dllpath, CharSet = CharSet.Ansi)]
		private static extern HF_STATUS LoadOBJ(
			string obj_paths,
			int group_method,
			float yrot,
			float xrot,
			float zrot,
			ref IntPtr out_mesh_info,
			ref int num_meshes
		);

		[DllImport(dllpath, CharSet = CharSet.Ansi)]
		internal static extern HF_STATUS StoreMesh(
			ref IntPtr out_info,
			[In] int[] indices,
			int num_indices,
			[In] float[] vertices,
			int num_vertices,
			string name,
			int id
		);

		[DllImport(dllpath, CharSet = CharSet.Ansi)]
		internal static extern HF_STATUS DestroyMeshInfo(
		   IntPtr MeshToDestroy
	   );

		[DllImport(dllpath, CharSet = CharSet.Ansi)]
		internal static extern HF_STATUS RotateMesh(IntPtr Meshinfo, float xrot, float yrot, float zrot);

		[DllImport(dllpath, CharSet = CharSet.Ansi)]
		internal static extern HF_STATUS DestroyMeshInfoPtrArray(IntPtr data_array);

		[DllImport(dllpath, CharSet = CharSet.Ansi)]
		internal static extern HF_STATUS GetMeshName(IntPtr MeshInfo, ref IntPtr out_name);

		[DllImport(dllpath, CharSet = CharSet.Ansi)]
		internal static extern HF_STATUS GetMeshID(IntPtr MeshInfo, ref int out_id);

		[DllImport(dllpath, CharSet = CharSet.Ansi)]
		internal static extern HF_STATUS GetVertsAndTris(IntPtr MI, ref IntPtr index_out, ref int num_triangles, ref IntPtr vertex_out, ref int num_vertices);

		[DllImport(dllpath, CharSet = CharSet.Ansi)]
		internal static extern HF_STATUS DestroyCharArray(IntPtr char_array);

	}
}