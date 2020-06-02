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
		std::vector<MeshInfo> LoadMeshObjects(
			std::vector<std::string> & path, 
			GROUP_METHOD gm = ONLY_FILE,
			bool change_coords = true
		);

		/// <summary>
		/// Load a list of vertices raw from the specified file. 
		/// </summary>
		std::vector<std::array<float, 3>> LoadRawVertices(std::string path);
	}
}


#endif