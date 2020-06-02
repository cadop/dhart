#include <graph_generator_private.h>

#include <iostream>
#include <omp.h>
#include <thread>

#include <Constants.h>
#include <graph_generator.h>
#include <unique_queue.h>
#undef min

using namespace HF::SpatialStructures;

typedef std::pair<int, int> pair;
typedef std::set<pair> set;

namespace HF {
	namespace AnalysisMethods {
		constexpr v3 down{ 0, 0, -1 };
		// Quick factorial oneliner
		inline int fact(int n) { return (n == 1 || n == 0) ? 1 : fact(n - 1) * n; }

		/// <summary>
		/// Create a set of every permution for 0-limit
		/// </summary>
		/// <param name="limit">Max number in the set</param>
		/// <returns>A list of all permutions for 0-limit</returns>
		std::set<std::pair<int, int>> permutations(int limit) {
			// Create a vector of all numbers between 1 and limit + 1, as well as their inverses
			std::vector<int> steps;
			steps.reserve(2 * limit);
			for (int i = 1; i < limit + 1; i++) { //NOTE: This can be optimized using indexing
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

		inline bool GraphGeneratorPrivate::WalkableCheck(const Node& position)
		{
			v3 testpos{ position[0], position[1], position[2] };
			return CheckRay(testpos, down, FLOORS);
		}

		GraphGeneratorPrivate::GraphGeneratorPrivate(GraphGenerator& host)
			: GG(host) {};

		Graph GraphGeneratorPrivate::BuildNetwork()
		{
			v3 start = v3{ HF::SpatialStructures::roundhf(GG.start[0]), roundhf(GG.start[1]), roundhf(GG.start[2]) };

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
				std::cerr << "Initial Floor Check failed. Start Point " << start << " is not over valid ground" << std::endl;
			return G;
		}

		bool GraphGeneratorPrivate::CheckStart(v3& start) {
			return CheckRay(start, down);
		}

		inline void GraphGeneratorPrivate::GeneratePotentialChildren(const Node& parent, const std::vector<pair>& directions, std::vector<Node>& out_children) {
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
			// Use the distance between parent and child as the maximum distance for the occlusion check
			auto dir = parent.directionTo(child);
			return GG.ray_tracer.FireOcclusionRay(parent.x, parent.y, parent.z, dir[0], dir[1], dir[2], parent.distanceTo(child));
		}

		inline bool GraphGeneratorPrivate::CheckFloor(const Node& parent, Node& child)
		{
			// If the ray hits, check that the child isn't lower than our maximum z offset
			if (CheckRay(child, down, HIT_FLAG::FLOORS)) {
				auto dstep = parent.z - child.z;
				auto ustep = child.z - parent.z;

				return (dstep < GG.downstep && ustep < GG.upstep);
			}

			return false;
		}

		bool GraphGeneratorPrivate::isUpSlope(const Node& n1, const Node& n2)
		{
			// Slope is rise/run
			const float run = sqrtf(powf((n1.x - n2.x), 2) + powf((n1.y - n2.y), 2));
			const float rise = n2.z - n1.z;
			const float calc_slope = atan2(rise, run) * (180.00f / static_cast<float>(M_PI));

			return calc_slope > -1 * GG.downslope && calc_slope < GG.upslope;
		}

		void GraphGeneratorPrivate::GetChildren(const Node& parent, const std::vector<Node>& possible_children, std::vector<Edge>& out_children)
		{
			for (auto& child : CheckChildren(parent, possible_children)) {
				const STEP is_connected = CheckConnection(parent, child);

				// If the node is connected Add it to out list of valid children
				if (is_connected != STEP::NOT_CONNECTED) {
					out_children.emplace_back(Edge(child, parent.distanceTo(child),is_connected));
				}
			}
		}


		std::vector<HF::SpatialStructures::Node> GraphGeneratorPrivate::CheckChildren(const HF::SpatialStructures::Node& parent, std::vector<HF::SpatialStructures::Node> children)
		{
			std::vector<Node> valid_children;
			for (auto& child : children) {
				if (CheckRay(child, down))
				{
					// Check to see if the new position will satisfy up and downstep restrictions
					auto dstep = parent[2] - child[2];
					auto ustep = child[2] - parent[2];

					if (dstep < GG.downstep && ustep < GG.upstep) {
						valid_children.push_back(child);
					}
					else {
					}
				}
			}
			return valid_children;
		}
		
		STEP GraphGeneratorPrivate::CheckConnection(const Node& parent, const Node& child)
		{
			// Create a copy of parent and child that can be modified
			auto node1 = parent;
			auto node2 = child;

			node1[2] += 0.001f;
			node2[2] += 0.001f;

			// Check if there is a direct connection
			if (!OcclusionCheck(node1, node2)) {
				if (abs(node1.z - node2.z) < 0.001f) return STEP::NONE;

				//TODO: This eventually may just return isupslope
				else if (isUpSlope(parent, child)) return STEP::NONE;

				return STEP::NOT_CONNECTED;
			}

			// Otherwise check for a step based connection
			else {
				STEP s = STEP::NONE;
				if (node1[2] > node2[2]) {
					node1[2] = node1[2] + GG.downstep;
					node2[2] = node2[2] + 0.001f;
					s = STEP::DOWN;
				}

				else if (node1[2] < node2[2]) {
					node1[2] = node1[2] + GG.upstep;
					node2[2] = node2[2] + 0.001f;
					s = STEP::UP;
				}

				else if (node1[2] == node2[2]) {
					node1[2] = node1[2] + GG.upstep;
					node2[2] = node2[2] + 0.001f;
					s = STEP::OVER;
				}

				if (!OcclusionCheck(node1, node2)) {
					return s;
				}

				//	addDenied(node1, node2, v3(1, 0.5, 0));
				return STEP::NOT_CONNECTED;
			}
		}

		void GraphGeneratorPrivate::CrawlGeom(UniqueQueue & todo)
		{
			if(todo.empty()) throw std::exception("Started with no start point!");
			
			// Set logic to determine directions
			const set init_directs = { pair(-1, -1), pair(-1, 0), pair(-1, 1), pair(0, -1), pair(0, 1), pair(1, -1), pair(1, 0), pair(1, 1) };
			set angle_directions = permutations(GG.max_step_connection);
			std::vector<pair> directions;
			std::set_union(init_directs.begin(), init_directs.end(), angle_directions.begin(), angle_directions.end(), std::back_inserter(directions));

			int num_nodes = 0;
			while (!todo.empty() && (num_nodes < GG.max_nodes || GG.max_nodes < 0)) {
				const auto parent = todo.pop();

				// Compute a list of valid edges
				std::vector<Edge> links;
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
			// Set logic to determine directions
			const set init_directs = { pair(-1, -1), pair(-1, 0), pair(-1, 1), pair(0, -1), pair(0, 1), pair(1, -1), pair(1, 0), pair(1, 1) };
			set angle_directions = permutations(GG.max_step_connection);

			const int max_threads = std::thread::hardware_concurrency();
			omp_set_num_threads(max_threads);

			std::vector<pair> directions;
			std::set_union(init_directs.begin(), init_directs.end(), angle_directions.begin(), angle_directions.end(), std::back_inserter(directions));

			directions.shrink_to_fit();
			std::vector<Edge> links;
			int num_nodes = 0;

			while (!todo.empty() && (num_nodes < GG.max_nodes || GG.max_nodes < 0)) {
				
				// If the todo list is big enough, go into parallel;
				if (todo.size() >= 100) {

					int to_do_count = todo.size();
					if (GG.max_nodes > 0)
						to_do_count = std::min(todo.size(), GG.max_nodes - num_nodes);

					// Get every node out of the queue
					auto to_be_done = todo.popMany(to_do_count);
					if (to_be_done.size() == 0)
						printf("Oh no");
					// Create output array
					std::vector<std::vector<Edge>> OutEdges(to_do_count);

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
							
							// Add to the queue if applicable and record the edge in the graph
							for (auto& e : OutEdges[i]) {
								todo.push(e.child);
								G.addEdge(to_be_done[i], e.child, e.score);
							}
							num_nodes++;
						}
					}
				}

				// Otherwise go in sequence
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

		inline void GraphGeneratorPrivate::ComputerParent(const Node& parent, const std::vector<pair>& directions, std::vector<Edge>& child_links)
		{
			std::vector<Node> potential_children;
			GeneratePotentialChildren(parent, directions, potential_children);
			GetChildren(parent, potential_children, child_links);
		}
	}
}