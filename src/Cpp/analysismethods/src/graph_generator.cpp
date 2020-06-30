#include <graph_generator.h>
#include <graph_generator_private.h>
#include <charconv>
///
/// \file		graph_generator.cpp
/// \brief		Contains implementation for the <see cref="HF::AnalysisMethods::GraphGenerator">GraphGenerator</cref>
///
///	\author		TBA
///	\date		26 Jun 2020

#include <iostream>

namespace HF {
	namespace AnalysisMethods {
		
		GraphGenerator::GraphGenerator(RayTracer::EmbreeRayTracer  & RT, int walkable_id, int obstacle_id) :
			walkable_surfaces(walkable_id), obstacle_surfaces(obstacle_id) {
			ray_tracer = RT;
		};

		SpatialStructures::Graph GraphGenerator::BuildNetwork(
			const v3 & start_point,
			const v3 & Spacing,
			int MaxNodes,
			float UpStep,
			float UpSlope,
			float DownStep,
			float DownSlope,
			int max_step_connections,
			int cores
		){

			max_nodes = MaxNodes; downstep = DownStep; upslope = UpSlope;
			downslope = DownSlope; max_step_connection = max_step_connections;
			upstep = UpStep; start = start_point;
			spacing = Spacing; core_count = cores;
			
			// Create the friend with a reference to this
			GraphGenerator & t = *this;
			GraphGeneratorPrivate gg2p(t);
			
			// Return the results of the friend
			return gg2p.BuildNetwork();
		}
	}
}