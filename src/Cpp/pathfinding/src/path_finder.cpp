///
///	\file		path_finder.cpp
///	\brief		Source file for BoostGraph, BoostGraphDeleter, as well as DistPred, and miscellaneous procedures
///
///	\author		TBA
///	\date		17 Jun 2020
///
#include <path_finder.h>

#include <vector>
#include <robin_hood.h>
#include<omp.h>
#include<thread>
#include<math.h>
#include<execution>
#include <memory>

#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/dijkstra_shortest_paths_no_color_map.hpp>
#include <boost/exception/exception.hpp>

#include <boost_graph.h>
#include <path.h>

using namespace HF::SpatialStructures;
using namespace HF::Pathfinding;
using std::vector;

namespace HF::Pathfinding {
	/// <summary>
	/// Holds a distance and predecessor matrix for the boost pathfinding algorithm
	/// </summary>
	struct DistPred {
		std::vector<float> distance;
		std::vector<vertex_descriptor> predecessor;

		DistPred() {};

		/// <summary>
		/// Resize both starting arrays  to n
		/// </summary>
		DistPred(int n) {
			distance.resize(n);
			predecessor.resize(n);
		}
	};

	/*!
		\brief Construct the shortest path from start to end using the given predecessor and distance vectors.

		\param start ID of the starting point.
		\param end ID of the end point.
		\param pred Predecessor matrix for the start node.
		\param distances Distance matrix for pred

		\todo Replace exception with an assert statment. It shouldn't be triggered unless
		there's a problem with this algorithm?
	*/
	inline Path ConstructShortestPathFromPred(
		int start,
		int end,
		const std::vector<size_t>& pred,
		const std::vector<float>& distances
	) {

		// Create a new path and add the end point. 
		Path p;
		p.AddNode(end, 0);

		// Return an empty path if there's no path from start to end
		// indicated by the predecessor's value for the current node
		// being the current node.
		int current_node = end;
		if (pred[current_node] == current_node) return Path{};

		float last_cost = distances[current_node];

		// Follow the predecessor matrix from end to start
		while (current_node != start) {

			// If this is triggered, there's something wrong with this algorithm
			// because the path suddenly has more nodes than there are in the
			// entire graph.
			if (p.size() > pred.size())
				throw std::exception("Path included more nodes than contaiend in the graph!");

			// Get the next node from the predecessor matrix
			int next_node = pred[current_node];

			// Get the current cost for traversing the graph so far.
			float current_cost = distances[next_node];

			// Subtract the last cost from the graph to get the cost 
			// of traversing this node
			float de_facto_cost = last_cost - current_cost;

			// Add this node to the list
			p.AddNode(next_node, de_facto_cost);

			// Save this node and cost as the last node/cost
			last_cost = current_cost;
			current_node = next_node;
		}

		// Flip the order of this since this algorithm generates it from end to start
		p.Reverse();
		return p;
	}

	/*!
		\brief Overload to call this with a distPred instead of the raw arrays.

		\param start Starting point of the path to generate.
		\param end End point of the path to generate.
		\param dist_pred DistPred containing the predecessor and distance matricies start.
	*/
	inline Path ConstructShortestPathFromPred(int start, int end, const DistPred& dist_pred) {
		return ConstructShortestPathFromPred(start, end, dist_pred.predecessor, dist_pred.distance);
	}

	/*!
		\brief Build a row of the distance and predecessor matrices for the given node ID.

		\param g the graph to build the predecessor and distance matrices for
		\param id The ID of the node to generate the predecessor matrix from

		\returns
		A DistPred filled with the distance and predecessor arrays for ID in g.
	*/
	inline DistPred BuildDistanceAndPredecessor(const graph_t& g, int id) {

		// Construct an isntance of DistPred that can hold every node in the graph
		int n = num_vertices(g);
		DistPred dist_pred(n);

		// Give boost a reference to the array in distpred
		auto pm = boost::predecessor_map(&dist_pred.predecessor[0]);
		vertex_descriptor start_vertex = vertex(id, g);

		// Fill distpred's distance and predecssor arrays with info from the graph
		dijkstra_shortest_paths_no_color_map(
			g,
			start_vertex,
			pm.distance_map(&dist_pred.distance[0]).weight_map(boost::get(&Edge_Cost::weight, g))
		);

		return dist_pred;
	}

	Path FindPath(BoostGraph* bg, int start_id, int end_id)
	{
		const graph_t& graph = bg->g;
		auto dist_pred = BuildDistanceAndPredecessor(graph, start_id);
		return ConstructShortestPathFromPred(start_id, end_id, dist_pred.predecessor, dist_pred.distance);
	}

	vector<Path> FindPaths( BoostGraph * bg, const vector<int> & start_points, const vector<int> & end_points)
	{
		// Get the graph from bg
		const graph_t& graph = bg->g;
		vector<Path> paths(start_points.size());
		
		// Generate predecessor matrices for every unique start point
		robin_hood::unordered_map<int, DistPred> dpm;
		for (int start_point : start_points) 
			if (dpm.count(start_point) == 0)
				dpm[start_point] = BuildDistanceAndPredecessor(graph, start_point);

		//Create shortestpaths for start_end pair
		for (int i = 0; i < start_points.size(); i++) {
			int start = start_points[i];
			int end = end_points[i];
			const auto& dist_pred = dpm[start];

			paths[i] = ConstructShortestPathFromPred(start, end, dist_pred);
		}
		return paths;
	}

	void InsertPathsIntoArray(
		const BoostGraph * bg,
		const std::vector<int>& start_points,
		const std::vector<int>& end_points,
		HF::SpatialStructures::Path** out_paths,
		HF::SpatialStructures::PathMember** out_path_members,
		int* out_sizes
	) {

		// Get graph from boost graph
		const graph_t& graph = bg->g;
		robin_hood::unordered_map<int, DistPred> dpm;

		// Use maximum number of cores for this machine 
		int core_count = std::thread::hardware_concurrency();
		int cores_to_use = std::min(core_count-1, static_cast<int>(start_points.size()));
		omp_set_num_threads(cores_to_use);


		// Generate predecessor matrices for every unique start point
		std::vector<int> start_copy = start_points;
		std::sort(std::execution::par_unseq, start_copy.begin(),start_copy.end());
		std::vector<int> unique_starts;
		std::unique_copy(start_copy.begin(), start_copy.end(), std::back_inserter(unique_starts));

		// Preallocate entries for the hash map
		for (auto uc : unique_starts)
			dpm.emplace(std::pair<int, DistPred>{uc, DistPred()});

		// Build Predecessors and distance matrices in parallel
	#pragma omp parallel for schedule(dynamic) if (unique_starts.size() > cores_to_use && cores_to_use > 4)
		for (int i = 0; i < unique_starts.size(); i++) {
			int start_point = unique_starts[i];
			dpm[start_point] = BuildDistanceAndPredecessor(graph, start_point);
		}

		// Create paths in parallel.
#pragma omp parallel for schedule(dynamic) if (cores_to_use > 4)
		for (int i = 0; i < start_points.size(); i++) {

			// Get the start and end point for this path
			int start = start_points[i];
			int end = end_points[i];

			// Get a reference to the distance and predecessor array for this start point. 
			const auto& dist_pred = dpm[start];

			// Construct the shortest path, store a point for it in out_paths at index i
			out_paths[i] = new Path(ConstructShortestPathFromPred(start, end, dist_pred));

			// Store a pointer to that path's PathMembers in out_path_members
			out_path_members[i] = out_paths[i]->GetPMPointer();

			// Store the size of the path's PathMembers in index i of out_sizes
			out_sizes[i] = out_paths[i]->size();

			// If the size of the path is zero, delete the path and set it's pointer
			// to a null pointer in the output array.
			if (out_sizes[i] == 0) {
				delete (out_paths[i]);
				out_paths[i] = nullptr;
			}
		}
	}

	std::unique_ptr<BoostGraph, BoostGraphDeleter> CreateBoostGraph(const HF::SpatialStructures::Graph& g) {
		return std::unique_ptr<BoostGraph, BoostGraphDeleter>(new BoostGraph(g));
	}

	void BoostGraphDeleter::operator()(BoostGraph* bg) const {
		delete bg;
	}
}
