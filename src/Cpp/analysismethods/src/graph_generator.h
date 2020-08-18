/// \file		graph_generator.h
/// \brief		Contains definitions for the <see cref="HF::GraphGenerator::GraphGenerator">GraphGenerator</see>
///
///	\author		TBA
///	\date		26 Jun 2020

#pragma once
#define _USE_MATH_DEFINES
#include <Node.h>
#include <Edge.h>
#include <Graph.h>
#include <set>
#include <embree_raytracer.h>


/*! \brief Generate a graph of accessible space from a given start point. 

	\see GraphGenerator for more details.
*/
namespace HF::GraphGenerator {
	using v3 = std::array<float, 3>;
	using v3d = std::array<double, 3>;
	class GraphGenerator;
	class UniqueQueue;

	/*! 
		\brief Determines which geometry the ray will collide with.
		
		\remarks 
		Used internally by the GraphGenerator to determine which type of geometry to collide with
		when casting a ray. 

	*/ 
	enum HIT_FLAG {
		/// Floors only.
		FLOORS = 1,
		/// Obstacles only.
		OBSTACLES = 2,
		/// Collide with floors and obstacles.
		BOTH = 3
	};

	/*! Calculate P(n,r) as an array with each unique permutaton of 2 values being a pair.  */
	std::set<std::pair<int, int>> permutations(int limit);

	/*! \brief Generate a graph of accessible space from a given start point.

		\details
		The Graph Generator maps out "accessible" space on a model from a given starting point. In graphs created
		by this algorithm, node represents a point in space that a human can occupy, and each edge between nodes 
		indicates that a human can traverse from one node to another node. The Graph Generator is a powerful tool 
		for analyzing space, since the graph or nodes it outputs can be used as input to all the analysis methods 
		offered by HumanFactors, allowing for it to be the starting point of other analysis methods within
		HumanFactors.
		
		\remarks
		This class serves as an interface to \link GraphGeneratorPrivate \endlink. All of the implementation 
		for the GraphGenerator is located in its friend class GraphGeneratorPrivate. 

		\note 
		All arguments are in meters for distances and degrees for angles
		unless otherwise specified. For all calculations, the Graph Generator assumes
		geometry is Z-Up i.e. upstep is how high the step is in the z-direction, ground
		checks are performed in the -z direction etc. 

		\todo Obstacle/Walkable geometry support.

		\see GraphGeneratorPrivate for details about the implementation. 

	*/
	class GraphGenerator
	{
	private:
		int walkable_surfaces;					///< Corresponds to the meshID of walkable surfaces in the Raytracer (Unused)
		int obstacle_surfaces;					///< Corresponds to the meshid of obstacle surfaces in the Raytracer (Unused)
		int core_count;							///< Number of cores to use in parallel. 
		RayTracer::EmbreeRayTracer ray_tracer;	///< Raytracer to use for all intersections 
		v3d spacing; ///< Spacing between nodes as a double to help with offset precisions
		v3 start;	///< Start point for graph generation

		float downstep;			///< Maximum step down that can be considered accessible.
		float upslope;			///< Maximum upward slope in degrees that can be considered accessible.
		float downslope;		///< Maximum downard slope in degrees that can be considered accessible.
		float upstep;			///< Maximum upward step that can be considered accessible.
		float step_height;		 ///< Unused.
		float floor_offset;		 ///< Unused.
		int max_step_connection; ///< Multiplier for children to generate from a single parent node.

		int max_nodes; ///< Maximum number of nodes to generate. If less than zero, generate nodes without a limit.

		friend class GraphGeneratorPrivate; ///< Private implementation for the graph generator. 

	public:
		/// <summary> Create a new Graph Generator. </summary>
		/// <param name="RT">
		/// The raytracer to use for this scene. Geometry for the Raytracer must be Z-Up.
		/// </param>
		/// <param name="walkable_id"> The Raytracer's ID for walkable geometry (Unused) </param>
		/*!
			\param obstacle_id ID to use for obstacle geometry (Unused)
		*/
		GraphGenerator(RayTracer::EmbreeRayTracer& RT, int walkable_id, int obstacle_id = -1);

		/// <summary> Generate a Graph of accessible space. </summary>
		/// <param name="start_point"> Starting point for the breadth first search. </param>
		/// <param name="spacing"> Spacing between each node. </param>
		/// <param name="MaxNodes">
		/// Maximum amount of nodes for generation. Set to -1 for no limit.
		/// </param>
		/// <param name="UpStep"> Maximum upward step that can be considered accessible. </param>
		/// <param name="UpSlope">
		/// Maximum upward slope in degrees that can be considered accessible.
		/// </param>
		/// <param name="DownSlope">
		/// Maximum downard slope in degrees that can be considered accessible.
		/// </param>
		/// <param name="max_step_connection">
		/// Multiplier for children to generate from a single parent node.
		/// </param>
		/// <param name="cores">
		/// Number of CPU cores to use for the calculation. A setting of -1 will use all available
		/// cores. A setting of 0 or 1 will disable parallel processing entirely.
		/// </param>
		/// <returns> A Graph of accessible space for the provided arguments. </returns>

		/*!
			\code
				// Requires #include "graph_generator.h"
				// For brevity

				using HF::Geometry::LoadMeshObjects;
				using HF::RayTracer::EmbreeRayTracer;
				using HF::GraphGenerator::GraphGenerator;
				using HF::SpatialStructures::Graph;

				// Prepare the file path for plane.obj, load the mesh objects into mesh
				auto mesh = LoadMeshObjects(plane_path);
				const std::string plane_path = "plane.obj";
				// Create a GraphGenerator using an EmbreeRayTracer.

				EmbreeRayTracer ray_tracer(mesh);
				// walkable_id = 0, obstacle_id defaults to -1
				GraphGenerator graph_generator = GraphGenerator(ray_tracer, 0);

				// Prepare the parameters for building a graph
				std::array<float, 3> starting_position = { 0.0, 0.0, 0.5 };		// position to begin analysis
				std::array<float, 3> node_spacing = { 0.02f, 0.02f, 0.02f };	// space between nodes

				const int maximum_nodes = 100000;								// maximum amount of nodes for generation

				float max_slope = 1.0;											// maximum allowed slope between two nodes
				float max_step_height = 1.0;									// maximum height for a step

				float min_slope = 1.0;											// minimum step allowed between nodes
																				// per parent node
				int max_step_connection = 1;									// influences how many potential children are generated
				int core_count = 1;												// how many CPU cores are used in calculation

																				//	defaults to -1 if not set and uses all available cores

																				//  0 or 1 disables parallel processing entirely
				Graph g = graph_generator.BuildNetwork(starting_position,
				// Create a graph using BuildNetwork
													   maximum_nodes,
													   max_slope,
													   min_slope,
													   node_spacing,
													   max_step_height,
													   max_step_connection,

													   core_count);
				g.Compress();
				std::cout << "Graph size " << g.size() << std::endl;

			\endcode
			`>>>Graph size 101270`
		*/
		SpatialStructures::Graph BuildNetwork(
			const v3& start_point,
			const v3d& Spacing,
			int MaxNodes,
			float UpStep,
			float UpSlope,
			float DownStep,
			float DownSlope,
			int max_step_connections,
			int cores = -1
		);
	};
};