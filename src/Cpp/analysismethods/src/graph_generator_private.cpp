///
/// \file		graph_generator_private.cpp
/// \brief		Contains implementation for the <see cref="HF::GraphGenerator::GraphGeneratorPrivate">GraphGeneratorPrivate</see> class
///
///	\author		TBA
///	\date		26 Jun 2020

#include <graph_generator_private.h>

#include <iostream>
#include <omp.h>
#include <thread>

#include <Constants.h>
#include <graph_generator.h>
#include <unique_queue.h>
#include <embree_raytracer.h>
#undef min

using namespace HF::SpatialStructures;
using std::vector;

typedef std::pair<int, int> pair;
typedef std::set<pair> set;



namespace HF::GraphGenerator {
	constexpr v3 down{ 0, 0, -1 };
	
	static const vector<pair> init_directs = {
		pair(-1, -1), pair(-1, 0), pair(-1, 1),
		pair(0, -1), pair(0, 1), pair(1, -1),
		pair(1, 0), pair(1, 1)
	};


	/// <summary>
	/// Create a set of every permutation for all numbers between 0 and limit.
	/// </summary>
	/// <param name="limit">Highest number in the set</param>
	/// <returns>A list of all permutations for numbers 0 to limit.</returns>
	/// \todo This can be optimized by using indexing instead of pushback.
	std::set<std::pair<int, int>> permutations(int limit) {
		// Create a vector of all numbers between 1 and limit + 1, as well as their inverses
		vector<int> steps;
		steps.reserve(2 * limit);

		for (int i = 1; i < limit + 1; i++) { 	
			steps.push_back(i);
			steps.push_back(-i);
		}

		std::set<std::pair<int, int>> perms;

		// Nest a for loop to capture every permutation
		for (int j : steps)
			for (int k : steps)
				if (abs(j) != abs(k))
					perms.emplace(pair(j, k));

		return perms;
	}
	
	/*
		\brief Create the set of directions to offset nodes in.
	*/
	inline vector<pair> CreateDirecs(int max_step_connections) {
		// A max_step_connections of 1 is just init_directs
		if (max_step_connections == 1) return init_directs;

		// Otherwise generate extra directions
		auto perms = permutations(max_step_connections);

		// Copy init_directs into our output array
		vector<pair> out_directions = init_directs;

		// Resize our output array to fit the new permutations, then fill it
		out_directions.resize(out_directions.size() + perms.size());
		std::move(perms.begin(), perms.end(), out_directions.begin() + init_directs.size());
	
		return out_directions;
	}

	inline bool GraphGeneratorPrivate::WalkableCheck(const Node& position)
	{
		// Create a copy of position so it isn't overridden by checkray
		// if the ray succeeds.
		v3 testpos{ position[0], position[1], position[2] };
		return CheckRay(testpos, down, FLOORS);
	}

	GraphGeneratorPrivate::GraphGeneratorPrivate(GraphGenerator& host)
		: GG(host) {};

	Graph GraphGeneratorPrivate::BuildNetwork()
	{
		// round the position of the start point
		v3 start = v3{
			roundhf(GG.start[0]), 
			roundhf(GG.start[1]),
			roundhf(GG.start[2])};

		// Only crawl geom if start collides
		UniqueQueue q;
		if (CheckStart(start)) {
			q.push(start);

			// Run in parallel if specified
			if (GG.core_count != 0 && GG.core_count != 1) {

				// Set the number of threads to core_count if it's greater than 1
				if (GG.core_count > 1)	omp_set_num_threads(GG.core_count);
				else omp_set_num_threads(std::thread::hardware_concurrency());

				CrawlGeomParallel(q);
			}
			else
				CrawlGeom(q);
		}
		else
			std::cerr << "Initial Floor Check failed. Start Point " 
			<< start << " is not over valid ground" << std::endl;

		// Return the generated graph
		return G;
	}

	bool GraphGeneratorPrivate::CheckStart(v3& start) {
		return CheckRay(start, down);
	}

	inline void GraphGeneratorPrivate::GeneratePotentialChildren(
		const Node& parent,
		const vector<pair>& directions,
		vector<Node>& out_children
	) {
		out_children.clear();

		//TODO: Speed gain can be achieved here by using indexing
		for (const auto & dir : directions) {
			auto i = dir.first; auto j = dir.second;
			const float x = roundhf(parent[0] + (i * GG.spacing[0]));
			const float y = roundhf(parent[1] + (j * GG.spacing[1]));
			const float z = roundhf(parent[2] + GG.spacing[2]);

			out_children.emplace_back(Node(x, y, z));
		}
	}

	inline bool GraphGeneratorPrivate::OcclusionCheck(const Node& parent, const Node& child)
	{
		// Use the distance between parent and child
		// as the maximum distance for the occlusion check
		auto dir = parent.directionTo(child);
		return GG.ray_tracer.FireOcclusionRay(
			parent.x,
			parent.y,
			parent.z,
			dir[0],
			dir[1],
			dir[2],
			parent.distanceTo(child)
		);
	}


	bool GraphGeneratorPrivate::isUpSlope(const Node& n1, const Node& n2)
	{
		// Slope is rise/run
		const float run = sqrtf(powf((n1.x - n2.x), 2) + powf((n1.y - n2.y), 2));
		const float rise = n2.z - n1.z;

		// Calculate the angle between rise and run.
		const float calc_slope = atan2(rise, run) * (180.00f / static_cast<float>(M_PI));

		// Check against downslope and upslope. 
		return calc_slope > -1 * GG.downslope && calc_slope < GG.upslope;
	}

	void GraphGeneratorPrivate::GetChildren(
		const Node& parent,
		const vector<Node>& possible_children,
		vector<Edge>& out_children
	) {
		// Iterate through every child in children
		for (auto& child : CheckChildren(parent, possible_children)) {
			// Determine step type (if any)
			const STEP is_connected = CheckConnection(parent, child);

			// If the node is connected Add it to out list of valid children
			if (is_connected != STEP::NOT_CONNECTED) {
				out_children.emplace_back(Edge(child, parent.distanceTo(child),is_connected));
			}
		}
	}


	vector<Node> GraphGeneratorPrivate::CheckChildren(const Node& parent, vector<Node> children)
	{
		vector<Node> valid_children;

		// iterate through every child in children
		for (auto& child : children) {
			if (CheckRay(child, down))
			{
				// Check to see if the new position will satisfy up and downstep restrictions
				auto dstep = parent[2] - child[2];
				auto ustep = child[2] - parent[2];

				if (dstep < GG.downstep && ustep < GG.upstep)
					valid_children.push_back(child);
			}
		}
		return valid_children;
	}
	
	STEP GraphGeneratorPrivate::CheckConnection(const Node& parent, const Node& child)
	{
		// Create a copy of parent and child that can be modified
		auto node1 = parent;
		auto node2 = child;

		// Offset them from the ground slightly
		node1[2] += 0.001f;
		node2[2] += 0.001f;

		// See if there's a direct line of sight between parent and child
		if (!OcclusionCheck(node1, node2)) {

			// If there is a direct line of sight, and they're on the same plane
			// then there is no step. 
			if (abs(node1.z - node2.z) < 0.001f) return STEP::NONE;

			// Check if we can traverse the slope from parent to child.
			else if (isUpSlope(parent, child)) return STEP::NONE;

			return STEP::NOT_CONNECTED;
		}

		// Otherwise check for a step based connectiom
		else {

			STEP s = STEP::NONE;

			// If parent is higher than child, then offset child downwards
			if (node1[2] > node2[2]) {
				node1[2] = node1[2] + GG.downstep;
				node2[2] = node2[2] + 0.001f;
				s = STEP::DOWN;
			}

			// If parent is lower than child, offset child upwards by upstep
			else if (node1[2] < node2[2]) {
				node1[2] = node1[2] + GG.upstep;
				node2[2] = node2[2] + 0.001f;
				s = STEP::UP;
			}

			// If they're on an equal plane then offset by upstep to see
			// if the obstacle can be stepped over.
			else if (node1[2] == node2[2]) {
				node1[2] = node1[2] + GG.upstep;
				node2[2] = node2[2] + 0.001f;
				s = STEP::OVER;
			}

			// If there is a line of sight then the nodes are connected
			// with the step type we calculated
			if (!OcclusionCheck(node1, node2)) {
				return s;
			}

			// If not, then there is no connection between these
			// nodes. 
			return STEP::NOT_CONNECTED;
		}
	}

	void GraphGeneratorPrivate::CrawlGeom(UniqueQueue & todo)
	{
		const auto directions = CreateDirecs(GG.max_step_connection);

		int num_nodes = 0;
		while (!todo.empty() && (num_nodes < GG.max_nodes || GG.max_nodes < 0)) {
			const auto parent = todo.pop();

			// Compute a list of valid edges
			vector<Edge> links;
			ComputerParent(parent, directions, links);

			// Add new nodes to the queue. It'll drop them if they
			// already were evaluated, or already existed on the queue
			for (auto edge : links)
				todo.push(edge.child);

			// Add new edges to the graph
			if (!links.empty())
				for (const auto& edge : links)
					G.addEdge(parent, edge.child, edge.score);

			++num_nodes;
		}
	}

	void GraphGeneratorPrivate::CrawlGeomParallel(UniqueQueue & todo)
	{
		auto directions = CreateDirecs(GG.max_step_connection);

		// Create a vector to use for edges
		vector<Edge> links;
		int num_nodes = 0;

		// Iterate through every node int the todo-list
		while (!todo.empty() && (num_nodes < GG.max_nodes || GG.max_nodes < 0)) {
			
			// If the todo list is big enough, go into parallel;
			if (todo.size() >= 100) {

				// Pop nodes from the todo list (If max_nodes will be exceeded, then
				// only pop as many as are left in max_nodes.)
				int to_do_count = todo.size();
				if (GG.max_nodes > 0)
					to_do_count = std::min(todo.size(), GG.max_nodes - num_nodes);

				// Get every node out of the queue
				auto to_be_done = todo.popMany(to_do_count);
	
				// If this happens, that means something is wrong with the algorithm
				// since to_be_done with a size of zero would just cause the outer
				// while loop to not run anymore.
				assert(to_be_done.size() > 0);

				// Create output array
				vector<vector<Edge>> OutEdges(to_do_count);

				// Compute valid children for every node in parallel.
				#pragma omp parallel
				{
					#pragma omp for schedule(dynamic)
					for (int i = 0; i < to_do_count; i++) {
						const Node n = to_be_done[i];
						ComputerParent(n, directions, OutEdges[i]);
					}
				}

				// Now out of parallel we add every node to to the todo list in order
				// and add the parent to the graph
				for (int i = 0; i < to_do_count; i++) {
					if (!OutEdges[i].empty()) {
						
						// Add this child to the queue and record the edge in the graph
						// The queue will automatically discard nodes that have already
						// been in it before.
						for (auto& e : OutEdges[i]) {
							todo.push(e.child);
							G.addEdge(to_be_done[i], e.child, e.score);
						}

						// Increment max_nodes
						num_nodes++;
					}
				}
			}

			// Otherwise go in sequence (identical to CrawlGeom)
			else {
				const Node parent = todo.pop();
				links.clear();

				ComputerParent(parent, directions, links);

				for (auto edge : links)
					todo.push(edge.child);

				if (!links.empty())
					for (auto & edge : links)
						G.addEdge(parent, edge.child, edge.score);

				++num_nodes;
				//progress++;
				//bar.progress(progress, max_nodes);
			}
		}
	}

	bool GraphGeneratorPrivate::CheckRay(v3& position, const v3& direction, HIT_FLAG flag)
	{
		// Switch Geometry based on hitflag
		switch (flag) {
		case HIT_FLAG::FLOORS: // Both are the same for now. Waiting on obstacle support
		case HIT_FLAG::OBSTACLES:
			return GG.ray_tracer.FireRay(position, direction);
			break;

		case HIT_FLAG::BOTH:
			return GG.ray_tracer.FireRay(position, direction);
		default:
			throw std::exception("Invalid CheckRay flag");
		}
	}

	bool GraphGeneratorPrivate::CheckRay(Node& position, const v3& direction, HIT_FLAG flag) {
		// Switch Geometry based on hitflag
		switch (flag) {

		// Floors and Obstacles are identical for now.
		case HIT_FLAG::FLOORS:
		case HIT_FLAG::OBSTACLES:
			return GG.ray_tracer.FireRay(
				position[0],
				position[1],
				position[2],
				direction[0],
				direction[1],
				direction[2],
				-1.0f,
				GG.walkable_surfaces
			);
			break;

		case HIT_FLAG::BOTH:
			return GG.ray_tracer.FireRay(
				position[0],
				position[1],
				position[2],
				direction[0],
				direction[1],
				direction[2]
			);
		default:
			throw std::exception("Invalid CheckRay flag");
		}
	}

	inline void GraphGeneratorPrivate::ComputerParent(
		const Node& parent,
		const vector<pair>& directions,
		vector<Edge>& child_links
	){
		vector<Node> potential_children;
		// Calculate the potential children.
		GeneratePotentialChildren(parent, directions, potential_children);
		
		// Fill child_links with valid edges from parent to children
		// in potential_children.
		GetChildren(parent, potential_children, child_links);
	}
}