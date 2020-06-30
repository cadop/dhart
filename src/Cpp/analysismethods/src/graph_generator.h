#pragma once
#define _USE_MATH_DEFINES
#include <Node.h>
#include <Edge.h>
#include <Graph.h>
#include <set>
#include <embree_raytracer.h>

namespace HF {
	namespace AnalysisMethods {
		using v3 = std::array<float, 3>;
		class GraphGenerator;
		class UniqueQueue;

		///< Determines which geometry the ray will collide with
		enum HIT_FLAG {
			FLOORS = 1,
			OBSTACLES = 2,
			BOTH = 3
		};

		///< Calculates P(n,r) as an array with each 2 values being a pair
		std::set<std::pair<int, int>> permutations(int limit);

		/// <summary>
		/// Generate a graph of all areas accessible from a given start point
		/// </summary>
		class GraphGenerator
		{
		private:
			int walkable_surfaces;					///< Corresponds to the meshID of walkable surfaces in the Raytracer
			int obstacle_surfaces;					///< Corresponds to the meshid of obstacle surfaces in the Raytracer
			int core_count;							///< How many cores we're using
			RayTracer::EmbreeRayTracer ray_tracer;	///< EmbreeRayTracer member

			///< >---OPTIONS---/// maybe should be a struct?
			v3 spacing;
			v3 start;

			float downstep;
			float upslope;
			float downslope;
			float upstep;

			int max_step_connection;

			float step_height;
			float floor_offset;

			int max_nodes;

			friend class GraphGeneratorPrivate; // Make our private implementation our friend
		
		public:
			/// <summary>
			/// Create a new Graph Generator
			/// </summary>
			/// <param name="RT">The raytracer to use for this scene</param>
			/// <param name="walkable_id">The Raytracer's ID for walkable geometry</param>
			/// <param name="obstacle_id">The Raytracer's ID for obstacle geometry</param>
			/// <param name="RT">The raytracer to use for this scene</param>

			/*!
				\code
					// Requires #include "graph_generator.h"

					// For brevity
					using HF::Geometry::LoadMeshObjects;
					using HF::RayTracer::EmbreeRayTracer;
					using HF::AnalysisMethods::GraphGenerator;

					// Prepare the file path for plane.obj, load the mesh objects into mesh
					const std::string plane_path = "plane.obj";
					auto mesh = LoadMeshObjects(plane_path);

					// Create a GraphGenerator using an EmbreeRayTracer.
					// walkable_id = 0, obstacle_id defaults to -1
					EmbreeRayTracer ray_tracer(mesh);
					GraphGenerator graph_generator = GraphGenerator(ray_tracer, 0);

					// Now you can use a GraphGenerator to build a graph (using BuildNetwork)
				\endcode
			*/
			GraphGenerator(RayTracer::EmbreeRayTracer & RT, int walkable_id, int obstacle_id = -1);

			/// <summary>
			/// Generate a graph based on the raytracer and the specified parameters
			/// </summary>
			/// <param name="start_point">Point to start the analysis at</param>
			/// <param name="spacing">Spacing between nodes</param>
			/// <param name="MaxNodes">Maximum amount of nodes for generation</param>
			/// <param name="UpStep">Maximum height for a step</param>
			/// <param name="UpSlope">Maximum allowed slope between two nodes</param>
			/// <param name="DownSlope">Minimum Step allowed between nodes</param>
			/// <param name="max_step_connection">Influences how many potential children are generated per parent node</param>
			/// <param name="cores"> How many cores to use for the calculation. A setting of -1 will use all available
			/// cores. A setting of 0 or 1 will disable parallel processing entirely.</param>
			/// <returns>A graph generated with the given settings</returns>

			/*!
				\code
					// Requires #include "graph_generator.h"

					// For brevity
					using HF::Geometry::LoadMeshObjects;
					using HF::RayTracer::EmbreeRayTracer;
					using HF::AnalysisMethods::GraphGenerator;
					using HF::SpatialStructures::Graph;

					// Prepare the file path for plane.obj, load the mesh objects into mesh
					const std::string plane_path = "plane.obj";
					auto mesh = LoadMeshObjects(plane_path);

					// Create a GraphGenerator using an EmbreeRayTracer.
					// walkable_id = 0, obstacle_id defaults to -1
					EmbreeRayTracer ray_tracer(mesh);
					GraphGenerator graph_generator = GraphGenerator(ray_tracer, 0);

					// Prepare the parameters for building a graph
					std::array<float, 3> starting_position = { 0.0, 0.0, 0.5 };		// position to begin analysis
					std::array<float, 3> node_spacing = { 0.02f, 0.02f, 0.02f };	// space between nodes

					const int maximum_nodes = 100000;								// maximum amount of nodes for generation

					float max_step_height = 1.0;									// maximum height for a step
					float max_slope = 1.0;											// maximum allowed slope between two nodes
					float min_slope = 1.0;											// minimum step allowed between nodes

					int max_step_connection = 1;									// influences how many potential children are generated
																					// per parent node

					int core_count = 1;												// how many CPU cores are used in calculation
																					//	defaults to -1 if not set and uses all available cores
																					//  0 or 1 disables parallel processing entirely

					// Create a graph using BuildNetwork
					Graph g = graph_generator.BuildNetwork(starting_position,
														   node_spacing,
														   maximum_nodes,
														   max_step_height,
														   max_slope,
														   min_slope,
														   max_step_connection,
														   core_count);

					std::cout << "Graph size " << g.size() << std::endl;
					g.Compress();
				\endcode

				`>>>Graph size 101270`
			*/
			SpatialStructures::Graph BuildNetwork(
				const v3& start_point,
				const v3& Spacing,
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
};