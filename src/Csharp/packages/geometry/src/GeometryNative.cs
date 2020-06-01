using HumanFactors.NativeUtils;
using System;
using System.Linq.Expressions;
using System.Runtime.InteropServices;
using HumanFactors.Exceptions;
using System.IO;
using System.Collections;
using System.Collections.Generic;
using System.Linq;
using System.CodeDom;
using System.ComponentModel;

namespace HumanFactors.Geometry
{

    /// <summary>
    /// Native methods for the Geometry namespace
    /// </summary>
    internal static class NativeMethods
    {

        /// <summary>
        /// Path to the HumanFactors dll copied from <see cref="NativeConstants"/>.
        /// </summary>
        const string dllpath = NativeConstants.DLLPath;

        /// <summary>
        /// Attempt to load the OBJ at the given filepath. If a rotation is specifed, the obj will be rotated after
        /// it is loaded.
        /// </summary>
        /// <param name="obj_path">Filepath for the obj to load</param>
        /// <param name="xrot"> Degrees to rotate the mesh on the z axis. </param>
        /// <param name="yrot"> Degrees to rotate the mesh on the y axis.</param>
        /// <param name="zrot"> Degrees to rotate the mesh on the z axis. </param>
        /// <returns> A pointer to a valid instance of meshinfo from C++ </returns>
        internal static IntPtr C_LoadOBJ(string obj_path, float xrot, float yrot, float zrot)
        {
            IntPtr out_ptr = new IntPtr();
            HF_STATUS res = LoadOBJ(obj_path, obj_path.Length, xrot, yrot, zrot, ref out_ptr);
            if (res == HF_STATUS.NOT_FOUND)
                throw new FileNotFoundException(obj_path + " did not lead to any file");
            else if (res == HF_STATUS.INVALID_MESH)
                throw new InvalidMeshException(obj_path + " did not lead to an obj file, or the obj file was invalid!");
            
            return out_ptr;
        }

        /// <summary>
        /// Store a mesh in managed memory.
        /// </summary>
        /// <returns>A pointer to the mesh in unmanaged memory</returns>
        /// <exception cref="InvalidMeshException">That did not represent a valid mesh</exception>
        internal static IntPtr C_StoreMesh(IEnumerable<float> vertices, IEnumerable<int> indices, string name, int id)
        {
            var vert_array = vertices.ToArray<float>();
            var ind_array = indices.ToArray<int>();
            int num_verts = vertices.Count();
            int num_inds = indices.Count();
            IntPtr out_ptr = new IntPtr();

            HF_STATUS res = StoreMesh(ref out_ptr, ind_array, num_inds, vert_array, num_verts, name, id);

            if (res == HF_STATUS.INVALID_MESH)
                throw new InvalidMeshException("That did not represent a valid mesh");

            return out_ptr;
        }

        /// <summary>
        /// Rotate this mesh in C++
        /// </summary>
        internal static void C_RotateMesh(IntPtr mesh, float xrot, float yrot, float zrot) => RotateMesh(mesh, xrot, yrot, zrot);

        /// <summary>
        /// Loads an OBJ file into meshinfo at the given instance 
        /// </summary>
        [DllImport(dllpath, CharSet = CharSet.Ansi)]
        private static extern HF_STATUS LoadOBJ(
            string obj_paths,
            int length,
            float yrot,
            float xrot,
            float zrot,
            ref IntPtr out_mesh_info
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
    }
}