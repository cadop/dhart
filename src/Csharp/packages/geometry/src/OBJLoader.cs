using System;

namespace HumanFactors.Geometry
{
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

            \throws System.IO.FileNotFoundException No file was found at  path.
			\throws HumanFactors.Exceptions.InvalidMeshException
			The file at the path was not a valid OBJ.
			
            \remarks
            Use the other overload if you want to use a CommonRotation on the mesh. 

			\internal 
				\todo Support group type
				\todo Support multiple meshes
			\endinternal

			\code
				// Loading a mesh from plane.obj with no rotation.
				MeshInfo MI = OBJLoader.LoadOBJ("plane.obj");
			\endcode
        */
        public static MeshInfo LoadOBJ(string path, float xrot = 0, float yrot = 0, float zrot = 0)
        {
            var mesh_ptr = NativeMethods.C_LoadOBJ(path, xrot, yrot, zrot);
            return new MeshInfo(mesh_ptr);
		}
		
		/*!
            \brief Load an obj from the OBJ file at the given filepath.

			\param path Path to a valid OBJ file.
			\param rotation Degrees to rotate the mesh in the x,y, and z direction after loading. 

			\returns An instance of MeshInfo containing the mesh loaded from the OBJ at path.

            \throws System.IO.FileNotFoundException No file was found at  path.
			\throws HumanFactors.Exceptions.InvalidMeshException
			The file at the path did not represent a valid OBJ.
			
            \remarks
			The rotation parameter can be used with the rotations in CommonRotations to easily
			perform common rotations on meshes. This is useful if you're loading a mesh that is
			Y-up and you need to convert it to Z-up for use in the GraohGenerator. 
		
			\internal 
				\todo Support group type
				\todo Support multiple meshes
			\endinternal

			\code
				// Loading a mesh from plane.obj with no rotation.
				MeshInfo MI = OBJLoader.LoadOBJ("plane.obj");
			\endcode
		*/
		public static MeshInfo LoadOBJ(string path, Vector3D rotation) => LoadOBJ(path, rotation.x, rotation.y, rotation.z);
    }
}