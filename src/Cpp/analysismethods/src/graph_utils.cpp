#include <graph_generator.h>

#include <HitStruct.h>
#include <constants.h>
#include <edge.h>
#include <embree_raytracer.h>
#include <ray_data.h>
#include <cassert>

namespace HF::GraphGenerator {

	/*! \brief Constant used as a direction for downwards raycasts. */
	static const real3 down = { 0,0,-1 };

	using HF::SpatialStructures::roundhf_tmp;
	using HF::SpatialStructures::trunchf_tmp;

	using HF::SpatialStructures::Node;
	using HF::SpatialStructures::Edge;
	using HF::SpatialStructures::STEP;

	using std::vector;

	using HF::RayTracer::HitStruct;
	/*! 
		\brief Convert a point_type to a node.
		
		\tparam point_type An array-like type that meets the pointlike concept
	
		\param ct x,y,z coordinates to create the new node with. 

		\returns A node containing the x,y,z components of `ct`
	
	*/
	template<typename point_type>
	inline Node ToNode(const point_type& ct) {
		return Node(ct[0], ct[1], ct[2]);
	}

	/*! 
		\brief Calculate the distance between two nodes

		\tparam n1_type A vector type that is indexable with square brackets for up to 3 elements
				(pointlike concept)
		\tparam n2_type n1_type A vector type that is indexable with square brackets for up to 3 elements
						(pointlike concept)

		\param n1 Location to calculate distance from
		\param n2 Location to calculate distance to

		\returns The distance from n1 to n2. 
	*/
	template<typename n1_type, typename n2_type>
	inline real_t DistanceTo(const n1_type& n1, const n2_type& n2) {
		return sqrt(pow((n1[0] - n2[0]), 2) + pow((n1[1] - n2[1]), 2) + pow((n1[2] - n2[2]), 2));
	}

	/*! 
		\brief Normalize a vector.
		
		\tparam vector_type Must be indexable with square brackets for up to 3 elements. (pointlike concept)
	
		\param v The vector to normalize

		\post v will be normalized to have a magnitude of 1.
	*/
	template<typename vector_type>
	inline void Normalize(vector_type& v) {
		auto magnitude = sqrt(pow(v[0], 2) + pow(v[1], 2) + pow(v[2], 2));
		v[0] = v[0] / magnitude;
		v[1] = v[1] / magnitude;
		v[2] = v[2] / magnitude;
	}

	/*! 
	
		\brief Calculate the normalized direction from one node to another
	
		\tparam vector_type The type of vectors used to hold the coordinates of `n1` and `n2.
							Each element should be indexable with square brackets up to 3 values.

		\param n1 Node to calcualte direction from
		\param n2 Node to calculate direction to

		\returns A vector_type containing the normalized direction `n1` to `n2`.
	*/
	template<typename vector_type>
	inline vector_type DirectionTo(const vector_type& n1, const vector_type& n2) {
		vector_type direction_vector;

		direction_vector[0] = n2[0] - n1[0];
		direction_vector[1] = n2[1] - n1[1];
		direction_vector[2] = n2[2] - n1[2];

		// Normalize the vector
		Normalize(direction_vector);
		return direction_vector;
	}

	///< Directions that are used always used by the graph generator. 
	static const vector<pair> init_directs = {
		pair(-1, -1), pair(-1, 0), pair(-1, 1),
		pair(0, -1), pair(0, 1), pair(1, -1),
		pair(1, 0), pair(1, 1)
	};

	optional_real3 ValidateStartPoint(RayTracer& RT, const real3& start_point, const GraphParams& Params)
	{
		return CheckRay(RT, start_point, down, Params.precision.node_z, HIT_FLAG::FLOORS, Params.geom_ids);
	}

	/*! 
		\brief Determine if a hit is against the geometry type specified
		
		\param goal Hitflag that the intersection is being cheecked against
		\param ID id of the mesh that was intesected
		\param geom_dict Rules to use for determining if the intersection was successful

		\returns True if the intersection abides by the rules in geom_dict, false otherwise

		\remarks
		This function serves as the sole place for intersection mesh ids to be checked using
		the ifnormation in geom_dict. 

	*/
	inline bool CheckGeometryID(HIT_FLAG goal, int id, const GeometryFlagMap & geom_dict) {
		
		// If the target is both or the geometry rules are set to NO_FLAG, all hits are counted as
		// being on walkable geometry
		if (geom_dict.Mode == GeometryFilterMode::ALL_INTERSECTIONS || goal == HIT_FLAG::BOTH)
			return true;
		
		// Otherwise do different checks based on the ruleset
		else if (geom_dict.Mode == GeometryFilterMode::OBSTACLES_ONLY)
			// If only obstacles are specified, this works like a blacklist/whitelist
			if (goal == OBSTACLES)
				return geom_dict[id] == HIT_FLAG::OBSTACLES;
			else
				return geom_dict[id] != HIT_FLAG::OBSTACLES;

		else if (geom_dict.Mode == GeometryFilterMode::OBSTACLES_AND_FLOORS)
			// In OBSTACLES_AND_FLOORS mode, the id's type must exactly match the goal
			return (goal == geom_dict[id]);
	}

	optional_real3 CheckRay(
		RayTracer& ray_tracer,
		const real3& origin,
		const real3& direction,
		real_t node_z_tolerance,
		HIT_FLAG flag,
		const GeometryFlagMap & geometry_dict)
	{
		// Setup default params
		HitStruct<real_t> res;

		// Cast the ray. On success, this returns the ID and distance to intersection.
		res = ray_tracer.Intersect(origin, direction);

		// Check if it hit and the ID of the geometry matches what we were looking for. 
		if (res.DidHit() && CheckGeometryID(flag, res.meshid, geometry_dict)) {
			// Create a new optional point with a copy of the origin
			optional_real3 return_pt(origin);

			// Move it in direction
			MoveNode(res.distance, direction, *return_pt);
			//double temp_diff = (origin[2]) - dist; // Sanity check for direction -1 to see influence of movenode arithmetic 
			// Round the position to the z value tolerance
			return_pt.pt[2] = HF::SpatialStructures::roundhf_tail<real_t>(return_pt.pt[2], 1/node_z_tolerance);

			// Return the optional point
			return return_pt;
		}

		// Otherwise, return an empty optional_real3 to signal that
		// the point didn't intersect
		return optional_real3();
	}

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

	vector<pair> CreateDirecs(int max_step_connections) {
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

	vector<graph_edge> GetChildren(
		const real3& parent,
		const vector<real3>& possible_children,
		RayTracer& rt,
		const GraphParams& GP
		)
	{
		std::vector<graph_edge> valid_edges;

		// Call CheckChildren to get rid of all children that aren't over valid ground or don't meet our upstep
		// and downstep requirements. This array of children will also be moved directly ontop of the ground their over.
		const auto checked_children = CheckChildren(parent, possible_children, rt, GP);

		// Iterate through every child in the checked children
		for (const auto& child : checked_children)
		{
			// Determine the type of connection between the parent and child
			//  including if it is a step, slope, or not connected
			const STEP connection_type = CheckConnection(parent, child, rt, GP);

			// If the node is connected Add it to out list of valid children
			if (connection_type != STEP::NOT_CONNECTED)
			{
				// Add the edge to the array of children, storing the distance and connection type
				valid_edges.emplace_back(graph_edge(ToNode(child), DistanceTo(parent, child), connection_type));
			}
		}
		return valid_edges;
	}

	std::vector<real3> CheckChildren(
		const real3& parent,
		const std::vector<real3>& possible_children,
		RayTracer& rt,
		const GraphParams& GP)
	{
		vector<real3> valid_children;

		// Iterate through every child in the set of possible children
		for (const auto& child : possible_children)
		{

			// Check if a ray intersects a mesh
			optional_real3 potential_child = CheckRay(rt, child, down, GP.precision.node_z, HIT_FLAG::FLOORS, GP.geom_ids);
			
			if (potential_child)
			{
				// Get a reference to the child in the return value now that we know it's over valid ground
				auto& confirmed_child = potential_child.pt;

				// TODO: this is a premature check and should be moved to the original calling function
				//      after the step type check since upstep and downstep are parameters for stepping and not slope

				// Check to see if the new position will satisfy up and downstep restrictions
				real_t dstep = parent[2] - confirmed_child[2];
				real_t ustep = confirmed_child[2] - parent[2];


				if (dstep < GP.down_step && ustep < GP.up_step)
					valid_children.push_back(confirmed_child);
			}
		}
		return valid_children;
	}

	bool OcclusionCheck(const real3& parent, const real3& child, RayTracer& RT)
	{
		// Use the distance between parent and child
		// as the maximum distance for the occlusion check
		return RT.Occluded(parent, DirectionTo(parent, child), DistanceTo(parent, child));
	}

	bool CheckSlope(const real3& parent, const real3& child, const GraphParams& gp)
	{
		// Slope is rise/run
		const real_t run = sqrt(pow((parent[0] - child[0]), 2) + pow((parent[1] - child[1]), 2));
		const real_t rise = child[2] - parent[2];

		// Calculate the angle between rise and run.
		const real_t calc_slope = atan2(rise, run) * (static_cast<real_t>(180.00) / static_cast<real_t>(M_PI));

		// Check against downslope and upslope.
		return calc_slope > -1.0 * gp.down_slope && calc_slope < gp.up_slope;
	}

	HF::SpatialStructures::EdgeSet CalculateStepType(const HF::SpatialStructures::Subgraph& sg, HF::RayTracer::MultiRT& rt) {
		// Step type will be stored here and returned from this function

		//Initialize output
		std::vector<HF::SpatialStructures::EdgeSet> edge_set;

		//From Subgraph sg
		Node parent_node = sg.m_parent;
		std::vector<Edge> edge_list = sg.m_edges;

		// We can preallocate this container to have edge_list.size()
		// blocks since we know how many children are in the subgraph.
		std::vector<HF::SpatialStructures::IntEdge> children(edge_list.size());
		auto it_children = children.begin();

		//For every edge...
		for (Edge link_a : edge_list) {
			Node curr_child = link_a.child;

			//Type Casting
			real_t parent_x = CastToReal(parent_node.x);
			real_t parent_y = CastToReal(parent_node.y);
			real_t parent_z = CastToReal(parent_node.z);

			real_t child_x = CastToReal(curr_child.x);
			real_t child_y = CastToReal(curr_child.y);
			real_t child_z = CastToReal(curr_child.z);

			std::vector<real_t> parent_loc = { parent_x, parent_y, parent_z };
			std::vector<real_t> child_loc = { child_x, child_y, child_z };

			real3 parent_cast = CastToReal3(parent_loc);
			real3 child_cast = CastToReal3(child_loc);

			//Get the step type between parent and child
			STEP step_type = CheckConnection(parent_cast, child_cast, rt);

			//Construct an intedge for this parent-child pair to add to edgeset
			HF::SpatialStructures::IntEdge ie = { link_a.child.id, static_cast<float>(step_type) };

			*(it_children++) = ie;
		}

		HF::SpatialStructures::EdgeSet es = { parent_node.id, children };

		return es;
	}

	std::vector<HF::SpatialStructures::EdgeSet> CalculateStepType(const HF::SpatialStructures::Graph& g, HF::RayTracer::MultiRT& rt) {
		// Retrieve all nodes from g so we can obtain subgraphs.
		std::vector<HF::SpatialStructures::Node> nodes = g.Nodes();

		// The result container will always be, at most, the node count of g. 
		// We can preallocate this memory so we do not have to resize during the loop below.
		std::vector<HF::SpatialStructures::EdgeSet> result(nodes.size());
		auto it_result = result.begin();

		for (Node parent_node : nodes) {
			// Get subgraph via parent_node
			HF::SpatialStructures::Subgraph sg = g.GetSubgraph(parent_node);

			// Call CalculateStepType using the subgraph
			HF::SpatialStructures::EdgeSet step_types = CalculateStepType(sg, rt);

			*(it_result++) = step_types;
		}
		return result;
	}

	void CalculateAndStoreStepType(HF::SpatialStructures::Graph& g, HF::RayTracer::MultiRT& rt) {
		// calculates and stores step types of all edges and stores it in the graph

		// Compression needed before adding edges of alternate cost
		g.Compress();

		// Get all edges with weights corresponding to step type
		auto result = HF::GraphGenerator::CalculateStepType(g, rt);

		// Add edges to the graph
		g.AddEdges(result, "step_type");
	}

	HF::SpatialStructures::STEP CheckConnection(
		const real3& parent,
		const real3& child,
		RayTracer& rt)
	{
		// Default graphh generator ground offset
		const auto GROUND_OFFSET = 0.01;

		// Create a copy of parent and child that can be modified
		auto node1 = parent;
		auto node2 = child;

		// Offset them from the ground slightly
		node1[2] += GROUND_OFFSET;
		node2[2] += GROUND_OFFSET;

		if (!OcclusionCheck(node1, node2, rt)) {
			// Case for when there is direct line of sight between nodes.
			// Since the edge is already in the graph, we do not need to check whether
			// they're on the same plane or if the slope between them is valid.
			return STEP::NONE;
		}
		// Otherwise check for step based connections.
		else {
			// Since these edges were already verified in the graph generator,
			// we do not need to check line of sight changes using offsets.
			STEP s = STEP::NONE;

			// If parent is higher than child, then we have a down step
			if (parent[2] > child[2]) {
				s = STEP::DOWN;
			}
			// If parent is lower than child, then we have an up step.
			else if (parent[2] < child[2]) {
				s = STEP::UP;
			}
			// If parent is on an equal plane with the child, then we step over an obstacle.
			else if (parent[2] == child[2]) {
				s = STEP::OVER;
			}
			return s;
		}

	}

	HF::SpatialStructures::STEP CheckConnection(
		const real3& parent,
		const real3& child,
		RayTracer& rt,
		const GraphParams& params)
	{
		// Get groundoffset from graph parameters
		const auto GROUND_OFFSET = params.precision.ground_offset;

		// Create a copy of parent and child that can be modified
		auto node1 = parent;
		auto node2 = child;

		// Offset them from the ground slightly
		node1[2] += GROUND_OFFSET;
		node2[2] += GROUND_OFFSET;

		// See if there's a direct line of sight between parent and child
		if (!OcclusionCheck(node1, node2, rt)) {
			// If there is a direct line of sight, and they're on the same plane
			// then there is no step.
			if (abs(node1[2] - node2[2]) < GROUND_OFFSET) return STEP::NONE;

			// If they are not on the same plane, it means this is a slope. Check if the slope is within the threshold.			
			else if (CheckSlope(parent, child, params)) return STEP::NONE;

			return STEP::NOT_CONNECTED;
		}

		// Otherwise check for a step based connectiom
		else {
			STEP s = STEP::NONE;
			// If parent is higher than child, the check is to go downstairs
			// Since the child is lower, raise the child height by the downstep limit
			// to be checked for a connection
			if (parent[2] > child[2]) 
			{
				node1 = child;
				node2 = parent;
				node1[2] = node1[2] + params.down_step;
				node2[2] = node2[2] + GROUND_OFFSET;
				s = STEP::DOWN;
			}

			// If parent is lower than child, the check is to go upstairs
			// Since the child is lower, raise the child height by the upstep limit
			// to be checked for a connection
			else if (node1[2] < node2[2]) 
			{
				node1 = parent;
				node2 = child;
				node1[2] = node1[2] + params.up_step;
				node2[2] = node2[2] + GROUND_OFFSET;
				s = STEP::UP;
			}

			// If they're on an equal plane then offset by upstep to see
			// if the obstacle can be stepped over.
			else if (node1[2] == node2[2]) 
			{
				node1 = parent;
				node2 = child;
				node1[2] = node1[2] + params.up_step;
				node2[2] = node2[2] + GROUND_OFFSET;
				s = STEP::OVER;
			}

			// If there is a line of sight then the nodes are connected
			// with the step type we calculated
			if (!OcclusionCheck(node1, node2, rt))
				return s;
		}

		// If not, then there is no connection between these
		// nodes.
		return STEP::NOT_CONNECTED;
	}

	bool CompareCheckConnections(HF::SpatialStructures::Graph& g, RayTracer& rt,
		const GraphParams& params, std::vector < HF::SpatialStructures::EdgeSet> to_compare)
	{
		const auto graph_nodes = g.Nodes();
		for (Node node : graph_nodes) {
			int parent_id = node.id;
			std::vector<HF::SpatialStructures::IntEdge> outgoing_edges = to_compare[parent_id].children;
			for (HF::SpatialStructures::IntEdge edge : outgoing_edges) {
				int child_id = edge.child;
				float step_type = edge.weight;
				Node child = g.NodeFromID(child_id);

				//Type Casting
				real_t parent_x = CastToReal(node.x);
				real_t parent_y = CastToReal(node.y);
				real_t parent_z = CastToReal(node.z);

				real_t child_x = CastToReal(child.x);
				real_t child_y = CastToReal(child.y);
				real_t child_z = CastToReal(child.z);

				std::vector<real_t> parent_loc = { parent_x, parent_y, parent_z };
				std::vector<real_t> child_loc = { child_x, child_y, child_z };

				real3 parent_cast = CastToReal3(parent_loc);
				real3 child_cast = CastToReal3(child_loc);

				float expected_step_type = static_cast<float>(CheckConnection(parent_cast, child_cast,
					rt, params));
				if (expected_step_type != 0 && expected_step_type != step_type) {
					return false;
				}
			}
		}
		return true;
	}
	std::vector<real3> GeneratePotentialChildren(
		const real3& parent,
		const std::vector<pair>& directions,
		const real3& spacing,
		const GraphParams& GP) 
	{
		// Create a vector of out_children to hold results
		vector<real3> out_children(directions.size());

		// Iterate through the set of all directions, then offset children in each direction
		for (int i = 0; i < directions.size(); i++) {
			// Get the direction at this index
			const auto& dir = directions[i];

			// Extract the x and y directions
			real_t x_offset = dir.first;
			real_t y_offset = dir.second;

			// Add the user-defined spacing to the x and y components of the parent.
			// Then round the result.
			const real_t x = roundhf_tmp<real_t>(std::fma(x_offset,spacing[0], parent[0]), GP.precision.node_spacing);
			const real_t y = roundhf_tmp<real_t>(std::fma(y_offset,spacing[1], parent[1]), GP.precision.node_spacing);
			// Round the z value to a lower precision assuming it helps embree
			const real_t z = roundhf_tmp<real_t>(parent[2] + spacing[2], GP.precision.node_z);

			// Add these new values as a node in the out_children list
			out_children[i] = real3{ x, y, z };
		}

		return out_children;
	}
}