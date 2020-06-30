///
/// \file		graph_generator.h
/// \brief		Contains definitions for the <see cref="HF::AnalysisMethods::GraphGenerator">GraphGenerator</cref>
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

namespace HF {
	namespace AnalysisMethods {
		using v3 = std::array<float, 3>;
		class GraphGenerator;
		class UniqueQueue;

		///< Determines which geometry the ray will collide with ///<
		enum HIT_FLAG {
			FLOORS = 1,
			OBSTACLES = 2,
			BOTH = 3
		};

		///< Calculates P(n,r) as an array with each 2 values being a pair ///<
		std::set<std::pair<int, int>> permutations(int limit);

		/// <summary>
		/// Generate a graph of all areas accessible from a given start point
		/// </summary>
		class GraphGenerator
		{
		private:
			int walkable_surfaces;					///< Corresponds to the meshID of walkable surfaces in the Raytracer ///<
			int obstacle_surfaces;					///< Corresponds to the meshid of obstacle surfaces in the Raytracer ///<
			int core_count;							///< How many cores we're using ///<
			RayTracer::EmbreeRayTracer ray_tracer;	///< EmbreeRayTracer member ///<

			///< >---OPTIONS---/// maybe should be a struct? ///<
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