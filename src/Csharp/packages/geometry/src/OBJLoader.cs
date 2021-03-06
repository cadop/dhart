using System;
using System.Text.RegularExpressions;

namespace DHARTAPI.Geometry
{


	/*! 
	  \brief Methods for seperating meshes when loading from OBJ
	
	*/
	public enum GROUP_METHOD{
		ONLY_FILE = 0, ///< Don't group submeshes. Return the entire file as a single MeshInfo.
		BY_GROUP = 1, ///< Create a seperate meshinfo for every obj group in the file.
		BY_MATERIAL = 2 ///< Create a seperate meshinfo for every material in the file. Requires the .mtl to be located next to the OBJ upon loading. If a .mtl could not be found, ONLY_FILE is used as a fallback.
	}

    /*! 
        \brief Load mesh geometry from OBJ files on disk.

        \remarks
        Internally is doing the same work as calling MeshInfo's constructor, so loaders
        for other filetypes can be created.
    */
    public static class OBJLoader
    {
        /*!
            \brief Load an obj from the OBJ file at the given filepath.

			\param path Path to a valid OBJ file.
			\param xrot Degrees to rotate the mesh on the x axis.
			\param yrot Degrees to rotate the mesh on the y axis.
			\param zrot Degrees to rotate the mesh on the z axis.
			
			\returns An instance of MeshInfo containing the mesh loaded from the OBJ at path.

            \throws System.IO.FileNotFoundException No file was found at path.
			\throws DHARTAPI.Exceptions.InvalidMeshException
			The file at the path was not a valid OBJ.
			
            \remarks
            Use the other overload if you want to use a CommonRotation on the mesh. 

			\code
				// Loading a mesh from plane.obj with no rotation.
				MeshInfo MI = OBJLoader.LoadOBJ("plane.obj");

			\endcode
        */
        public static MeshInfo LoadOBJ(string path, float xrot = 0, float yrot = 0, float zrot = 0)
        {
            var mesh_ptr = NativeMethods.C_LoadOBJ(path, xrot, yrot, zrot, GROUP_METHOD.ONLY_FILE);
            return new MeshInfo(mesh_ptr[0]);
		}

		/*!
			\brief Load an obj from the OBJ file at the given filepath.

			\param path Path to a valid OBJ file.
			\param gm Method of grouping the OBJ into submeshes
			\param xrot Degrees to rotate the mesh on the x axis.
			\param yrot Degrees to rotate the mesh on the y axis.
			\param zrot Degrees to rotate the mesh on the z axis.

			\returns All submeshes in the obj file at `path` grouped by `gm`

			\throws System.IO.FileNotFoundException No file was found at  path.
			\throws DHARTAPI.Exceptions.InvalidMeshException
			The file at the path was not a valid OBJ.
			
			\see GROUP_METHOD to see the different ways of grouping the different geometry in a .obj file

			\par Example
			\snippet geometry\test_geometry.cs EX_LoadSubmeshes

			```
			Name: arcs_floor, ID: 0, Verts: 150, Triangles: 192
			Name: arcs_03, ID: 1, Verts: 750, Triangles: 1344
			Name: object32, ID: 2, Verts: 3190, Triangles: 4784
			Name: object31, ID: 3, Verts: 3454, Triangles: 5264
			Name: pillar_cor, ID: 4, Verts: 3478, Triangles: 5312
			```
			\code
			\endcode
		*/
		public static MeshInfo[] LoadOBJSubmeshes(string path, GROUP_METHOD gm, float xrot = 0, float yrot = 0, float zrot = 0)
        {
			// Load from native memory
            IntPtr[] mesh_ptrs = NativeMethods.C_LoadOBJ(path, xrot, yrot, zrot, gm);

			// Iterate through each submesh and create new MeshInfo objects for each.
			int num_meshes = mesh_ptrs.Length;
			MeshInfo[] return_meshes = new MeshInfo[num_meshes];
			for (int i = 0; i < num_meshes; i++)
				return_meshes[i] = new MeshInfo(mesh_ptrs[i]);

			return return_meshes;
		}

		/*!
            \brief Load an obj from the OBJ file at the given filepath.

			\param path Path to a valid OBJ file.
			\param rotation Degrees to rotate the mesh in the x,y, and z direction after loading. 

			\returns An instance of MeshInfo containing the mesh loaded from the OBJ at path.

            \throws System.IO.FileNotFoundException No file was found at  path.
			\throws DHARTAPI.Exceptions.InvalidMeshException
			The file at the path did not represent a valid OBJ.
			
            \remarks
			The rotation parameter can be used with the rotations in CommonRotations to easily
			perform common rotations on meshes. This is useful if you're loading a mesh that is
			Y-up and you need to convert it to Z-up for use in the GraohGenerator. 
		
			\endinternal

			\code
				// Loading a mesh from plane.obj with no rotation.
				MeshInfo MI = OBJLoader.LoadOBJ("plane.obj");
			\endcode
		*/
		public static MeshInfo LoadOBJ(string path, Vector3D rotation) => LoadOBJ(path, rotation.x, rotation.y, rotation.z);
    }
}