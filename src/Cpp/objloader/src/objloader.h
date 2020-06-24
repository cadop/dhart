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

		/*!
			\code
				// be sure to #include "objloader.h", and #include <vector>

				// relative path begins where EXE file is located
				// if file_path is not a path to a valid OBJ file, HF::Exceptions::FileNotFound is thrown
				std::string file_path = "big_teapot.obj";

				// meshvec is a vector of meshinfo from file_path
				std::vector<HF::Geometry::MeshInfo> meshvec = HF::Geometry::LoadMeshObjects(file_path, HF::Geometry::GROUP_METHOD::ONLY_FILE, false);

				// retrieve the MeshInfo
				HF::Geometry::MeshInfo info = meshvec[0];
			\endcode
		*/
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
		
		/*!
			\code
				// be sure to #include "objloader.h", and #include <vector>

				// Prepare the file paths
				const auto PATH_0 = "big_teapot.obj";
				const auto PATH_1 = "plane.obj";
				const auto PATH_2 = "sibenik.obj";

				// PATH_0, PATH_1, and PATH_2 each represent a path to an OBJ file.
				// Note that if any path in pathvec is invalid, HF::Exceptions::FileNotFound will be thrown
				// when LoadMeshObjects is called
				std::vector<std::string> pathvec = { PATH_0, PATH_1, PATH_2 };

				// The overload for LoadMeshObjects is called for each member (which is an OBJ file path) in pathvec,
				// then IDs are reassigned for each MeshInfo object within the std::vector<MeshInfo> that is returned
				std::vector<HF::Geometry::MeshInfo> meshvec = HF::Geometry::LoadMeshObjects(pathvec, HF::Geometry::GROUP_METHOD::ONLY_FILE, false);

				std::cout << "Total loaded: " << meshvec.size() << std::endl;

				// Print IDs of all mesh info
				for (auto mesh_info : meshvec) {
					std::cout << "Mesh ID: " << mesh_info.GetMeshID() << std::endl;
				}
			\endcode
		*/
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

		/*!
			\code
			// be sure to #include "objloader.h", and #include <vector>

			// Note that filepath must lead to a valid OBJ file, or
			// HF::Exceptions::InvalidOBJ will be thrown when Geometry::LoadMeshObjects is called
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
	}
}

#endif /// OBJLOADER_H
