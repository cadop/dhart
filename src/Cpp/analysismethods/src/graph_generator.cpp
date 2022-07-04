///
/// \file		graph_generator.cpp
/// \brief		Contains implementation for the functions in the GraphGenerator class.
///	\author		TBA
///	\date		26 Jun 2020

#define NOMINMAX
#include <graph_generator.h>
#include <Constants.h>
#include <node.h>
#include <Edge.h>
#include <graph.h>
#include <robin_hood.h>
#include <omp.h>

#include <unique_queue.h>

#include <iostream>
#include <thread>

using HF::SpatialStructures::Graph;
using HF::SpatialStructures::Node;
using HF::SpatialStructures::Edge;
using HF::SpatialStructures::roundhf_tmp;
using HF::SpatialStructures::trunchf_tmp;

using std::vector;
using HF::GraphGenerator::GeometryFlagMap;

namespace HF::GraphGenerator{ 
	/*! \brief Sets the core count of OpenMP

		\param cores Number of cores to use. If -1, use every as many cores as possible 
	*/
	inline void SetupCoreCount(int cores = -1) {
		if (cores > 1)	omp_set_num_threads(cores);
		else omp_set_num_threads(std::thread::hardware_concurrency());
	}

	/*! \brief Converts the raytracer to a multiRT if required, then map geometry ids to hitflags 
		
		\param gg Pointer to the graph generator to update
		\param obs_ids array of geometry ids to set as obstacles
		\param walk_ids Array of geometry ids to set as walkable

		\post `gg` will have it's geometry map populated with the passed geometry ids and it's raytracer
			  set to a MultiRT containing `gg`
	*/
	template <typename raytracer_type>
	inline void setupRT(GraphGenerator* gg, raytracer_type & rt, const std::vector<int> & obs_ids, const std::vector<int> & walk_ids ) {
		gg->ray_tracer = HF::RayTracer::MultiRT(&rt);
		gg->params.geom_ids.SetGeometryIds(obs_ids, walk_ids);
	}

	GraphGenerator::GraphGenerator(HF::RayTracer::EmbreeRayTracer & rt, const vector<int> & obstacle_ids, const vector<int> & walkable_ids){
		setupRT<HF::RayTracer::EmbreeRayTracer> (this, rt, obstacle_ids, walkable_ids);
	}

	GraphGenerator::GraphGenerator(HF::RayTracer::NanoRTRayTracer & rt, const vector<int> & obstacle_ids, const vector<int> & walkable_ids) {
		setupRT<HF::RayTracer::NanoRTRayTracer> (this, rt, obstacle_ids, walkable_ids);
	}

	GraphGenerator::GraphGenerator(HF::RayTracer::MultiRT & ray_tracer, const vector<int> & obstacle_ids, const vector<int> & walkable_ids)
	{
		this->ray_tracer = ray_tracer;
		this->params.geom_ids.SetGeometryIds(obstacle_ids, walkable_ids);
	}

	SpatialStructures::Graph GraphGenerator::IMPL_BuildNetwork(
		const real3& start_point,
		const real3& Spacing,
		int MaxNodes,
		real_t UpStep,
		real_t UpSlope,
		real_t DownStep,
		real_t DownSlope,
		int max_step_connections,
		int cores,
		real_t node_z_precision,
		real_t node_spacing_precision,
		real_t ground_offset)
	{
		if (ground_offset < node_z_precision)
		{
			std::cerr << "Ground offset is less than z-precision. Setting node offset to Z-Precision." << std::endl;
			ground_offset = node_z_precision;
			//std::logic_error("Ground offset is less than Z-Precision");
		}

		// Store parameters in params
		params.down_step = DownStep; params.up_step = UpStep;
		params.up_slope = UpSlope;	params.down_slope = DownSlope;

		params.precision.node_z = node_z_precision;
		params.precision.node_spacing = node_spacing_precision;
		params.precision.ground_offset = ground_offset;

		// Store class members
		this->max_nodes = MaxNodes;
		this->spacing = Spacing; 
		this->core_count = cores;
		this->max_step_connection = max_step_connections;
		
		// Take the user defined start point and round it to the precision
		// that the dhart package can handle. 
		real3 start = real3{
		  roundhf_tmp<real_t>(start_point[0], params.precision.node_spacing),
		  roundhf_tmp<real_t>(start_point[1], params.precision.node_spacing),
		  roundhf_tmp<real_t>(start_point[2], params.precision.node_z) 
		};

		// Define a queue to use for determining what nodes need to be checked
		UniqueQueue to_do_list;
		optional_real3 checked_start = ValidateStartPoint(ray_tracer, start, this->params);

		// Check if the start raycast connected.
		if (checked_start)
		{
			// Overwrite start with the checked start point
			start = *checked_start;

			// add it to the to-do list
			to_do_list.PushAny(start);

			// If no connection was found, return an empty graph
			if (this->core_count != 0 && this->core_count != 1)
			{
				SetupCoreCount(this->core_count);
				return CrawlGeomParallel(to_do_list);
			}
			// Run the single core version of the graph generator
			else
				return CrawlGeom(to_do_list);
		}
		else
			return Graph();
	}

	Graph GraphGenerator::CrawlGeomParallel(UniqueQueue& todo)
	{
		// Generate the set of directions to use for each set of possible children
		// Uses the maximum connection defined by the user
		const auto directions = CreateDirecs(max_step_connection);

		// Initialize a tracker for the number of nodes that can be compared to the max nodes limit
		int num_nodes = 0;

		RayTracer & rt_ref = this->ray_tracer;

		Graph G;
		// Iterate through every node int the todo-list while it does not reach the maximum number of nodes limit
		while (!todo.empty() && (num_nodes < max_nodes || max_nodes < 0))
		{
			// Pop nodes from the todo list (If max_nodes will be exceeded, then
			// only pop as many as are left in max_nodes.)
			int to_do_count = todo.size();
			if (max_nodes > 0)
				to_do_count = std::min(todo.size(), max_nodes - num_nodes);

			// Get as many nodes as possible out of the queue
			auto to_be_done = todo.popMany(to_do_count);

			// If this happens, that means something is wrong with the algorithm
			// since to_be_done with a size of zero would just cause the outer
			// while loop to not run anymore.
			assert(to_be_done.size() > 0);

			// Create array arrays of edges that will be used to store results
			vector<vector<Edge>> OutEdges(to_do_count);

			// Compute valid children for every node in parallel.
			#pragma omp parallel for schedule(dynamic) if (to_be_done.size() > 100)
			for (int i = 0; i < to_do_count; i++)
			{
				// Get the parent node from the todo list at index i
				// and cast it to a real3
				const Node& n = to_be_done[i];
				const auto real_parent = CastToReal3(n);

				// Generate children around this parent
				const std::vector<real3> children = GeneratePotentialChildren(
					real_parent,
					directions,
					spacing,
					params
				);

				// Calculate valid edges for this parent and store them in
				// the edges array at it's index
				OutEdges[i] = GetChildren(
					real_parent,
					children,
					rt_ref,
					params
				);
			}

			// Go through each edge array we just calculated, then 
			// add the edges to the graph and todo list in sequence
			for (int i = 0; i < to_do_count; i++) {

				// Only continue if there are edges for this node
				if (!OutEdges[i].empty()) {

					// Iterate through each edge and add it to the graph / todolist
					for (const auto& e : OutEdges[i]) {
						todo.push(e.child);
						G.addEdge(to_be_done[i], e.child, e.score);
					}
					
					// Increment max nodes
					num_nodes++;
				}
			}
		}

		return G;
	}

	Graph GraphGenerator::CrawlGeom(UniqueQueue& todo)
	{
		// Create directions
		const auto directions = CreateDirecs(this->max_step_connection);

		// Cast this to a reference to avoid dereferencing every time
		RayTracer& rt_ref = this->ray_tracer;

		int num_nodes = 0;
		Graph G;
		while (!todo.empty() && (num_nodes < this->max_nodes || this->max_nodes < 0)) {

			// Get the parent node from the todo list
			const auto parent = todo.pop();

			// To maintain our precision standards, we'll convert this node to a real3.
			const auto real_parent = CastToReal3(parent);

			// Generate children from parent
			const std::vector<real3> children = GeneratePotentialChildren(
				real_parent,
				directions, 
				spacing,
				params
			);

			// Create edges
			std::vector<graph_edge> edges = GetChildren(
				real_parent,
				children,
				rt_ref,
				params
			);

			// Add new nodes to the queue. It'll drop them if they
			// already were evaluated, or already existed on the queue
			for (auto edge : edges)
				todo.PushAny(edge.child);

			// Add new edges to the graph
			if (!edges.empty())
				for (const auto& edge : edges)
					G.addEdge(parent, edge.child, edge.score);

			// Increment node count
			++num_nodes;
		}
		return G;
	}
}