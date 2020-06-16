///
///	\file		objloader.h
///	\brief		Header file for functions in the HF::Geometry namespace
///
///	\author		TBA
///	\date		16 Jun 2020
///
#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <vector>
#include <string>

namespace HF
{
	namespace Geometry
	{
		enum GROUP_METHOD {
			ONLY_FILE = 0,
			BY_GROUP = 1,
			BY_MATERIAL = 2,
			MATERIAL_AND_FILE = 3
		};

		class MeshInfo;

		/// <summary>
		/// Load obj file into mesh info instances
		/// </summary>
		/// <param name="path">The path to a valid OBJ file</param>
		/// <param name="gm">How to divide the mesh into subobjects</param>
		/// <param name="change_coords">Convert OBJ coordinate system to Rhino's coordinates</param>
		/// <returns> A vector of meshinfo from the given file</returns>
		/// <exception cref="HF::Exceptions::InvalidOBJ"> Thrown if the obj at path fails to load</exception>

		/// \code{.cpp}
		///	std::string file_path = "directory/filename.obj";
		/// // if file_path is not a path to a valid OBJ file, HF::Exceptions::FileNotFound is thrown
		///
		/// std::vector<MeshInfo> meshvec = Geometry::LoadMeshObjects(file_path, GROUP_METHOD::BY_MATERIAL, false);
		/// // meshvec is a vector of meshinfo from file_path
		/// \endcode
		std::vector<MeshInfo> LoadMeshObjects(
			std::string path,
			GROUP_METHOD gm = ONLY_FILE, 
			bool change_coords = true
		);
		
		/// <summary>
		/// Load obj file into mesh info instances
		/// </summary>
		/// <param name="path">The path to a valid OBJ file</param>
		/// <param name="gm">How to divide the mesh into subobjects</param>
		/// <param name="change_coords">Convert OBJ coordinate system to Rhino's coordinates</param>
		/// <returns> A vector of meshinfo from the given file</returns>
		/// <exception cref="HF::Exceptions::InvalidOBJ"> Thrown if the obj at path fails to load</exception>
		
		/// \code{.cpp}
		/// std::vector<std::string> pathvec = { PATH_0, PATH_1, PATH_2 };
		/// // PATH_0, PATH_1, and PATH_2 each represent a path to an OBJ file.
		/// // Note that if any path in pathvec is invalid, HF::Exceptions::FileNotFound will be thrown
		/// // when LoadMeshObjects is called 
		///
		/// std::vector<MeshInfo> meshvec = Geometry::LoadMeshObjects(pathvec, GROUP_METHOD::MATERIAL_AND_FILE, false);
		/// // The overload for LoadMeshObjects is called for each member (which is an OBJ file path) in pathvec,
		/// // then IDs are reassigned for each MeshInfo object within the std::vector<MeshInfo> that is returned
		/// \endcode
		std::vector<MeshInfo> LoadMeshObjects(
			std::vector<std::string> & path, 
			GROUP_METHOD gm = ONLY_FILE,
			bool change_coords = true
		);

		/// <summary>
		/// Load a list of vertices raw from the specified file.
		/// <param name="path">The path for the desired file from which vertices will be loaded</param>
		/// </summary>
		/// <returns>a vector of array (type float, size 3) that represents the vertices from the file specified at path</returns>

		/// \code{.cpp}
		/// std::string filepath = PATH_TO_FILE;
		/// // Note that filepath must lead to a valid OBJ file, or
		/// // HF::Exceptions::InvalidOBJ will be thrown when Geometry::LoadMeshObjects is called
		///
		/// std::vector<std::array<float, 3>> vertices = Geometry::LoadRawVertices(filepath);
		/// // vertices now contains the raw vertices loaded from the OBJ file specified at filepath
		/// \endcode
		std::vector<std::array<float, 3>> LoadRawVertices(std::string path);
	}
}

#endif /// OBJLOADER_H
