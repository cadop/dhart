#include <graph_generator.h>

#include <Constants.h>
#include <Edge.h>
#include <embree_raytracer.h>
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
		return CheckRay(RT, start_point, down, Params.precision.node_z);
	}

	optional_real3 CheckRay(
		RayTracer& ray_tracer,
		const real3& origin,
		const real3& direction,
		real_t node_z_tolerance,
		HIT_FLAG flag)
	{
		// Setup default params
		double dist = -1.0;
		int id = -1.0f;
		bool res = false;

		// Switch Geometry based on hitflag
		switch (flag) {
		case HIT_FLAG::FLOORS: // Both are the same for now. Waiting on obstacle support
		case HIT_FLAG::OBSTACLES:
			res = ray_tracer.FireAnyRayD(origin, direction, dist, id);
			break;
		case HIT_FLAG::BOTH:
			res = ray_tracer.FireAnyRayD(origin, direction, dist, id);
			break;
		default:
			assert(false);
		}

		// If successful, make a copy of the node, move it, then return it
		if (res) {
			// Create a new optional point with a copy of the origin
			optional_real3 return_pt(origin);

			// Move it in direction
			MoveNode(dist, direction, *return_pt);
			// double temp_diff = (origin[2]) - dist; // Sanity check for direction -1 to see influence of movenode arithmetic 
			// Truncate the Z value before leaving this function
			// This is for clarity, since the node was already modified
			return_pt.pt[2] = trunchf_tmp<real_t>(return_pt.pt[2], node_z_tolerance);

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
			optional_real3 potential_child = CheckRay(rt, child, down, GP.precision.node_z);
			
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
		return RT.FireAnyOcclusionRay(parent, DirectionTo(parent, child), DistanceTo(parent, child));
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
			if (node1[2] > node2[2]) {
				node1[2] = node1[2] + params.down_step;
				node2[2] = node2[2] + GROUND_OFFSET;
				s = STEP::DOWN;
			}

			// If parent is lower than child, the check is to go upstairs
			// Since the child is lower, raise the child height by the upstep limit
			// to be checked for a connection
			else if (node1[2] < node2[2]) {
				node1[2] = node1[2] + params.up_step;
				node2[2] = node2[2] + GROUND_OFFSET;
				s = STEP::UP;
			}

			// If they're on an equal plane then offset by upstep to see
			// if the obstacle can be stepped over.
			else if (node1[2] == node2[2]) {
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
			const real_t x = roundhf_tmp<real_t>(parent[0] + (x_offset * spacing[0]), GP.precision.node_spacing);
			const real_t y = roundhf_tmp<real_t>(parent[1] + (y_offset * spacing[1]), GP.precision.node_spacing);
			// Round the z value to a lower precision assuming it helps embree
			const real_t z = roundhf_tmp<real_t>(parent[2] + spacing[2], GP.precision.node_z);

			// Add these new values as a node in the out_children list
			out_children[i] = real3{ x, y, z };
		}

		return out_children;
	}
}