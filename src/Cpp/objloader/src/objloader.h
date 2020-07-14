///
///	\file		objloader.h
/// \brief		Contains definitions for the <see cref="HF::Geometry">Geometry</see> namespace
///
///	\author		TBA
///	\date		16 Jun 2020

#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>
#include <string>

/*!
	\brief 
	Eigen a C++ template library for linear algebra: matrices, vectors, numerical solvers,
	and related algorithms.

	\details
	HumanFactors uses Eigen to manage matricies in multiple areas. HF::SpatialStructures::Graph
	uses Eigen to manage its CSR, and HF::Geometry::MeshInfo uses Eigen to store and
	transform vertices for geometry.

	For more infomration on Eigen please see it's official website:
	http://eigen.tuxfamily.org/index.php?title=Main_Page

*/
namespace Eigen {}


/*!
	\brief Manipulate and load geometry from disk. 

	\details
	HF::Geometry contains all functionality in HumanFactors for dealing with mesh Geometry. The fundamental
	datatype of HF::Geometry is MeshInfo, which stores the vertices and indices of meshes as matricies
	in Eigen.

	\remarks
	Right now only OBJ files are supported for loading geometry from disk, however MeshInfo offers 
	multiple constructors that accept arrays of vertices and indices so support for more formats 
	can be developed. 

	\see LoadMeshObjects to load OBJ files from disk.
	\see MeshInfo for details on how HumanFactors represents Mesh geometry.
*/
namespace HF::Geometry{
	/*!
		\brief Method of grouping submeshes in OBJ files.
	
		\see LoadMeshObjects to use this enum.
	*/
	enum GROUP_METHOD {
		ONLY_FILE = 0, ///< Treat all the geometry in the file as a single mesh.
		BY_GROUP = 1, ///< Create a new MeshInfo instance for every OBJ group in the file.
		BY_MATERIAL = 2, ///< Create a new MeshInfo instance for every different material in the file.
		MATERIAL_AND_FILE = 3 ///< UNIMPLEMENTED
	};

	class MeshInfo;

	/// <summary> Create MeshInfo instances from the OBJ at path. </summary>
	/// <param name="path"> Path to the OBJ to load.</param>
	/// <param name="gm"> Method for dividing the mesh into subobjects. </param>
	/// <param name="change_coords"> Rotate the mesh from Y-up to Z-up. </param>
	/// <returns> A vector of meshinfo from the file at path. </returns>
	/*!
		\exception HF::Exceptions::InvalidOBJ The file at path was not a valid OBJ file.
		\exception HF::Exceptions::FileNotFound No file could be found at path.
		\exception std::exception gm did not match any valid GROUP_METHOD.

		\todo the name of the mesh should probably just be the filename if the group type
		is ONLY_FILE.
		\todo Definitely some areas for minor speed gains by using indexing instead of .push_bcaks
		\todo Change std::exception to std::out_of_range.
		\todo Refactor this to break out whatever code can be repeated or reused. This has a McCabe complexity
		of 25!!!!


		\see GROUP_METHOD for more information about every different group method. 
		\code
			// be sure to #include "objloader.h", and #include <vector>

			// relative path begins where EXE file is located if file_path is not a path to a
			// valid OBJ file, HF::Exceptions::FileNotFound is thrown
			std::string file_path = "big_teapot.obj";

			// meshvec is a vector of meshinfo from file_path
			std::vector<HF::Geometry::MeshInfo> meshvec = HF::Geometry::LoadMeshObjects(file_path,
			HF::Geometry::GROUP_METHOD::ONLY_FILE, false);

			// retrieve the MeshInfo
			HF::Geometry::MeshInfo info = meshvec[0];
		\endcode
	*/
	std::vector<MeshInfo> LoadMeshObjects(
		std::string path,
		GROUP_METHOD gm = ONLY_FILE,
		bool change_coords = false
	);

	/// <summary> Create MeshInfo instances from the OBJ files in path. </summary>
	/// <param name="path"> Path to the OBJ to load.</param>
	/// <param name="gm"> Method for dividing the mesh into subobjects. </param>
	/// <param name="change_coords"> Rotate the mesh from Y-up to Z-up. </param>
	/// <returns> A vector of meshinfo loaded from the files at path. </returns>
	/*!

		\exception HF::Exceptions::InvalidOBJ One or more of the files in paths was not a valid OBJ file.
		\exception HF::Exceptions::FileNotFound Could not find one or more of the files at path.
		\exception std::exception gm did not match any valid GROUP_METHOD.

		\code
			// be sure to #include "objloader.h", and #include <vector>

			// Prepare the file paths
			const auto PATH_0 = "big_teapot.obj";
			const auto PATH_1 = "plane.obj";
			const auto PATH_2 = "sibenik.obj";

			// PATH_0, PATH_1, and PATH_2 each represent a path to an OBJ file. Note that if any
			// path in pathvec is invalid, HF::Exceptions::FileNotFound will be thrown when
			// LoadMeshObjects is called
			std::vector<std::string> pathvec = { PATH_0, PATH_1, PATH_2 };

			// The overload for LoadMeshObjects is called for each member (which is an OBJ file
			// path) in pathvec, then IDs are reassigned for each MeshInfo object within the
			// std::vector<MeshInfo> that is returned
			std::vector<HF::Geometry::MeshInfo> meshvec = HF::Geometry::LoadMeshObjects(pathvec, HF::Geometry::GROUP_METHOD::ONLY_FILE, false);

			std::cout << "Total loaded: " << meshvec.size() << std::endl;

			// Print IDs of all mesh info
			for (auto mesh_info : meshvec) {
				std::cout << "Mesh ID: " << mesh_info.GetMeshID() << std::endl;
			}
		\endcode
	*/
	std::vector<MeshInfo> LoadMeshObjects(
		std::vector<std::string>& path,
		GROUP_METHOD gm = ONLY_FILE,
		bool change_coords = false
	);

	/*!
		\brief Load a list of vertices directly from an OBJ file.
		
		\param path Location of the OBJ to load on disk.
		
		\returns A vector of x,y,z positions for each vertex of each face of the OBJ.
		Every 3 vertices form a single triangle. on the mesh. 

		\remarks This is an alternative to the function that returns an instance of MeshInfo.
		Mostly for testing, since it uses a vertex array instead of an index array, making the
		end product take up more space in memory. 

		\exception HF::Exceptions::InvalidOBJ path did not lead to a valid OBJ file. 

		\todo Check if the file exists then thow file not found exception instead of invalidobj.

		\code
		// be sure to #include "objloader.h", and #include <vector>

		// Note that filepath must lead to a valid OBJ file, or HF::Exceptions::InvalidOBJ will
		// be thrown when Geometry::LoadMeshObjects is called
		std::string filepath = "plane.obj";

		// Vertices now contains the raw vertices loaded from the OBJ file specified at filepath
		std::vector<std::array<float, 3>> vertices = HF::Geometry::LoadRawVertices(filepath);

		// Display the vertices from filepath
		std::cout << "Vertices from " << filepath << ": " << std::endl;
		for (auto vertex : vertices) {
			std::cout << "(" << vertex[0] << ", " << vertex[1] << ", " << vertex[2] << ")" << std::endl;
		}
		\endcode
	*/
	std::vector<std::array<float, 3>> LoadRawVertices(std::string path);
	

	/*!
		\brief Get the path to the OBJ with the given key.
	
		\param key The name of the obj to get the path of

		\returns
		The path to the obj in key.

		\throws std::out_of_range if the given key couldn't be found
	*/
	std::string GetTestOBJPath(std::string key);
}

#endif /// OBJLOADER_H
