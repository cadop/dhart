///
///	\file		path_finder.cpp
/// \brief		Contains implmentation for the <see cref="HF::Pathfinding::BoostGraphDeleter">BoostGraphDeleter</see> structure
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
#include <boost/math/special_functions/fpclassify.hpp>

#include <boost_graph.h>
#include <path.h>

using namespace HF::SpatialStructures;
using namespace HF::Pathfinding;
using std::vector;

namespace HF::Pathfinding {
	/*! 
		\brief A single row of a distance and predecessor matrix.
		
		\details
		DistPred holds the distance and predecessor arrays for a single node, all the information
		needed to generate a path for the node it was generated from.

		\remarks
		This struct can be useful for keeping the multiple distance and predecessor arrays
		organized when preallocating them, as well as reducing the amount of code required
		to generate a path.

		\see ConstructShortestPathFromPred for an example of using these arrays to find
		the shortest path between two nodes.
	*/
	struct DistPred {
		std::vector<float> distance; ///< Distance array.
		std::vector<vertex_descriptor> predecessor;///< Predecessor array.

		/*! 
			\brief Construct a new instance of DistPred with empty distance and predecessor arrays.
		*/
		DistPred() {};

		/// <summary> Allocate enough space for n elements. </summary>
		/// \param n Size to allocate for both the distance and predecessor arrays. \remarks This is
		/// useful for calling boost's dijkstra methods since they expect the arrays to already be
		/// the correct size.
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

		// Return an empty path if there's no path from start to end indicated by the predecessor's
		// value for the current node being the current node.
		int current_node = end;
		if (pred[current_node] == current_node) return Path{};

		float last_cost = distances[current_node];

		// Follow the predecessor matrix from end to start
		while (current_node != start) {

			// If this is triggered, there's something wrong with this algorithm because the path
			// suddenly has more nodes than there are in the entire graph.
			if (p.size() > pred.size())
				throw std::exception("Path included more nodes than contaiend in the graph!");

			// Get the next node from the predecessor matrix
			int next_node = pred[current_node];

			// Get the current cost for traversing the graph so far.
			float current_cost = distances[next_node];

			// Subtract the last cost from the graph to get the cost of traversing this node
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
		\brief Build a row of the distance and predecessor matrices for the node at id.

		\param g Graph to build the predecessor and distance matrices for.
		\param id The node to generate the predecessor and distance matrix for.

		\returns
		A DistPred containing the distance and predecessor arrays for ID in g.
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
		// Get a reference to the graph contained by this boost graph
		const graph_t& graph = bg->g;
		
		// Build the distance and predecessor arrays for the node at start_id
		auto dist_pred = BuildDistanceAndPredecessor(graph, start_id);

		// Construct the shortest path using the predecessor and distance arrays
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

		// Copy and sort the input array of starting points
		std::vector<int> start_copy = start_points;
		std::sort(std::execution::par_unseq, start_copy.begin(),start_copy.end());
		
		// Remove all duplicates, effectively creating an array of unique start ids
		std::vector<int> unique_starts;
		std::unique_copy(start_copy.begin(), start_copy.end(), std::back_inserter(unique_starts));

		// Preallocate entries for the hash map in sequence so we don't corrupt the heap trying to
		// add elements in parallel.
		for (auto uc : unique_starts)
			dpm.emplace(std::pair<int, DistPred>{uc, DistPred()});

		// Build predecessor and distance matrices for each unique start point in parallel
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

			// If the size of the path is zero, delete the path and set it's pointer to a null
			// pointer in the output array.
			if (out_sizes[i] == 0) {
				delete (out_paths[i]);
				out_paths[i] = nullptr;
			}
		}
	}

	DistanceAndPredecessor GenerateDistanceAndPred(const BoostGraph& bg)
	{
		const auto & g = bg.g;

		// Generate distance and predecessor matricies
		const int num_nodes = bg.p.size();
		DistanceAndPredecessor out_distpred(num_nodes);
		
		// Iterate through every row in the array
	#pragma omp parallel for schedule(dynamic)
		for (int row = 0; row < num_nodes; row++) {
	
			// Get pointers to the beginning of the row for both matricies
			float* dist_row_start = out_distpred.GetRowOfDist(row);
			int* pred_row_start = out_distpred.GetRowOfPred(row);

			// Give boost a reference to the array in distpred
			auto pm = boost::predecessor_map(pred_row_start);

			// Get the descriptor of the starting vertex
			vertex_descriptor start_vertex = vertex(row, g);

			// calculate the shortest path using the row of the distance and predecessor arrays.
			// Boost should fill these when calculating the distance/predecessor matricies
			dijkstra_shortest_paths_no_color_map(
				g,
				start_vertex,
				pm.distance_map(dist_row_start).weight_map(boost::get(&Edge_Cost::weight, g))
			);

			// The documentation of boost kind of lies here. An infinite cost is not infinite,
			// as their pseudo code would lead you to believe. Actually it's setting the value
			// of the distance matrix for paths that can`t be created to the value
			// of std::numeric_limits<float>::max. This should be a NAN for the sake of readability. 
			for (int i = 0; i < num_nodes; i++)
			{
				//float & dist_element = dist_row_start[i];
				//int & pred_element = pred_row_start[i];

				// If the predecessor element is not not in the range of nodes,
				// this is boost trying to signal to us that there is no connection
				// between these nodes, so put a NAN there. 
				if (dist_row_start[i] == std::numeric_limits<float>::max()) {
					pred_row_start[i] = -1; // Ints have no way of representing NaN
					dist_row_start[i] = -1;
				}
			}
		}

		return out_distpred;
		
	}

	std::unique_ptr<BoostGraph, BoostGraphDeleter> CreateBoostGraph(const Graph& g, const std::string & cost_type) {
		// Construct a new boost graph using g, then wrap it in a unique pointer. 
		return std::unique_ptr<BoostGraph, BoostGraphDeleter>(new BoostGraph(g, cost_type));
	}

	void BoostGraphDeleter::operator()(BoostGraph* bg) const {
		delete bg;
	}

	void InsertAllToAllPathsIntoArray(BoostGraph* bg, Path** out_paths, PathMember** out_path_members, int* out_sizes) {
		size_t node_count = bg->p.size();
		size_t max_path = node_count * node_count;

		std::vector<int> start_points(max_path);
		std::vector<int> end_points(max_path);

		int curr_id = 0;

		// Populate the start points,
		// size will be (node_count)^2
		for (int i = 0; i < node_count; i++) {
			for (int k = 0; k < node_count; k++) {
				start_points[curr_id++] = i;
			}
		}

		curr_id = 0;

		// Populate the end points,
		// size will be (node_count)^2
		for (int i = 0; i < node_count; i++) {
			for (int k = 0; k < node_count; k++) {
				end_points[curr_id++] = k;
			}
		}

		/*
			The idea is that
				start_points[0] and end_points[0]
				will be the path from 0 to 0,

				start_points[1] and end_points[1]
				will be the path from 0 to 1

				start_points[2] and end_points[2]
				will be the path from 0 to 2,

				...etc...
		*/

		// Run InsertPathsIntoArray and mutate out_paths, out_path_members, and out_sizes
		InsertPathsIntoArray(bg, start_points, end_points, out_paths, out_path_members, out_sizes);
	}



	// ****************************************************************************************
	// Start of the new pathfinding functions
	// *																						*

	///*

	inline std::vector<DistPred> BuildDistanceAndPredecessorFast(const BoostGraph& bg) {

		// Get the graph from bg
		const graph_t& g = bg.g;
		const int num_nodes = bg.p.size();


		// Generate predecessor matrices for every unique start point
		//robin_hood::unordered_map<int, DistPred> dpm;

		std::vector<DistPred> pred(num_nodes);

		for (int row = 0; row < num_nodes; row++)
		{
			//if (dpm.count(row) == 0)
			//{
			// Construct an isntance of DistPred that can hold every node in the graph
			int n = num_vertices(g);
			DistPred dist_pred(n);

			// Give boost a reference to the array in distpred
			auto pm = boost::predecessor_map(&dist_pred.predecessor[0]);
			vertex_descriptor start_vertex = vertex(row, g);

			// Fill distpred's distance and predecssor arrays with info from the graph
			dijkstra_shortest_paths_no_color_map(
				g,
				start_vertex,
				pm.distance_map(&dist_pred.distance[0]).weight_map(boost::get(&Edge_Cost::weight, g))
			);
			//dpm[row] = dist_pred;
			pred[row] = dist_pred;
			//}
		}
		return pred;
	}
	//*/

	/*!
		\brief Construct the shortest path of node ids given predecessor and distance vectors.

		\param start ID of the starting point.
		\param end ID of the end point.
		\param pred Predecessor matrix for the start node.
		\param distances Distance matrix for pred

		\todo Replace exception with an assert statment. It shouldn't be triggered unless
		there's a problem with this algorithm?
	*/
	inline std::vector<int> ConstructShortestPathNodesFromPred(
		int start,
		int end,
		const std::vector<vertex_descriptor>& pred
	) {

		// Create a new path and add the end point.
		std::vector<int> path;
		path.push_back(end);

		// Return an empty path if there's no path from start to end indicated by the predecessor's
		// value for the current node being the current node.
		int current_node = end;
		if (pred[current_node] == current_node) return std::vector<int>{};

		// Follow the predecessor matrix from end to start
		while (current_node != start) {
			// Get the next node from the predecessor matrix
			int next_node = pred[current_node];
			if (next_node == -1) return std::vector<int>{};
			// Add this node to the list
			path.push_back(next_node);
			// Save this node and cost as the last node/cost
			current_node = next_node;
		}

		// Flip the order of this since this algorithm generates it from end to start
		std::reverse(path.begin(), path.end());
		return path;
	}

	std::vector<std::vector<int>> FindAPSP(BoostGraph* bg)
	{
		//std::vector<std::vector<std::vector<int>>>
		// 
		// Get the graph from bg
		const graph_t& graph = bg->g;

		std::vector<int> paths();

		//DistanceAndPredecessor matricies = GenerateDistanceAndPredFast(*bg);
		std::vector<DistPred> matricies = BuildDistanceAndPredecessorFast(*bg);

		const int num_nodes = bg->p.size();
		// This breaks down the full row of a predecessor 
		// Generate predecessor matrices for every unique start point
		robin_hood::unordered_map<int, DistPred> dpm;
		for (int row = 0; row < num_nodes; row++) {
			if (dpm.count(row) == 0)
				dpm[row] = BuildDistanceAndPredecessor(graph, row);
		}



		std::cout << "Generated Distance and Predecessor Matricies" << std::endl;
		//auto pred = matricies.pred;
		//std::vector<int> pathNodes();
		//std::vector<int> pathLengths();

		int numNodes = bg->p.size();
		std::vector<std::vector<int>> allPaths(numNodes*numNodes, std::vector<int>(numNodes));

	//#pragma omp parallel for schedule(dynamic)
		for (int i = 0; i < numNodes; i++) {
			for (int j = 0; j < numNodes; j++) {
				std::vector<int> path = std::vector<int>{};
				if (i!=j) {
					//int* curRow = matricies.GetRowOfPred(i);
					const auto& curRow = matricies[i].predecessor;
					path = ConstructShortestPathNodesFromPred(i, j, curRow);
				}
				int curIndx = (i * numNodes) + j; // basically the stride is being counted 
				
				// Add to the overall path
				allPaths[curIndx] = path;

			}
		}

		return allPaths;
	}

	inline DistanceAndPredecessor GenerateDistanceAndPredFast(const BoostGraph& bg)
	{
		const graph_t& g = bg.g;

		// Generate distance and predecessor matricies
		const int num_nodes = bg.p.size();
		DistanceAndPredecessor out_distpred(num_nodes);

		// Iterate through every row in the array
    //#pragma omp parallel for schedule(dynamic)
		for (int row = 0; row < num_nodes; row++) {

			// Get pointers to the beginning of the row for both matricies
			float* dist_row_start = out_distpred.GetRowOfDist(row);
			int* pred_row_start = out_distpred.GetRowOfPred(row);

			// Give boost a reference to the array in distpred
			auto pm = boost::predecessor_map(pred_row_start);

			// Get the descriptor of the starting vertex
			vertex_descriptor start_vertex = vertex(row, g);

			// calculate the shortest path using the row of the distance and predecessor arrays.
			// Boost should fill these when calculating the distance/predecessor matricies
			dijkstra_shortest_paths_no_color_map(
				g,
				start_vertex,
				pm.distance_map(dist_row_start).weight_map(boost::get(&Edge_Cost::weight, g))
			);
		}

		return out_distpred;

	}



}
