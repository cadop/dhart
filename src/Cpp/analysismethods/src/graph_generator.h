/// \file		graph_generator.h
/// \brief		Contains declarations for all functions related to the graph generator.
///
///	\author		TBA
///	\date		26 Jun 2020

#pragma once
#define _USE_MATH_DEFINES

#include <cmath>
#include <set>
#include <vector>
#include <array>
#include <Node.h>
#include <cassert>
#include <variant>
#include <MultiRT.h>
#include <unordered_map>

// Forward declares for embree raytracer.
namespace HF::RayTracer {
	class EmbreeRayTracer;
	class NanoRTRayTracer;
}
namespace HF::SpatialStructures {
	class Graph;
	struct Edge;
	enum STEP;
}

/*! \brief Generate a graph of accessible space from a given start point. 
	
	\details
	The Graph Generator maps out "accessible" space on a model from a given starting point. In graphs created
	by this algorithm, node represents a point in space that a human can occupy, and each edge between nodes
	indicates that a human can traverse from one node to another node. The Graph Generator is a powerful tool
	for analyzing space, since the graph or nodes it outputs can be used as input to all the analysis methods
	offered by HumanFactors, allowing for it to be the starting point of other analysis methods within
	HumanFactors.

	\par Obstacle Support
	The Graph Generator supports marking specific geometry as walkable or obstacles. Obstacle surfaces  are surfaces
	that the graph generator is not allowed to generate nodes on, while walkable surfaces are the only surfaces that the
	graph generator is permitted to generate nodes on. Depending on what arguments are first passed to the graph generator,
	it will use different rules for determining which of the inputs are obstacles and which are not. When no geometry
	ids are specified, all geometry is considered walkable. When only obstacle surfaces are specified, all geometry
	other than that in the obstacles array are considered walkable. If both an obstacle and walkable array are specified
	then obstacles will be considered inaccessible, walkable surfaces will be accessible, and all geometry not in
	either array will be considered not traversable.

	\note
	All arguments are in meters for distances and degrees for angles
	unless otherwise specified. For all calculations, the Graph Generator assumes
	geometry is Z-Up i.e. upstep is how high the step is in the z-direction, ground
	checks are performed in the -z direction etc.

	\see GraphGenerator for more details.
*/
namespace HF::GraphGenerator {

	/* For C++ 2020.
	template <typename T>
	concept Point = requires(T pt) {
			{pt[0]}->std::convertible_to<float>;
			{pt[1]}->std::convertible_to<float>;
			{pt[2]}->std::convertible_to<float>;
	};
	*/

	class UniqueQueue;
	struct optional_real3;

	using real_t = double;							  ///< Internal decimal type of the graph generator
	using real3 = std::array<real_t, 3>;			  ///< Type used for the standard coordinate set of the graph generator.
	using graph_edge = HF::SpatialStructures::Edge;   ///< Type of edge for the graph generator to use internally

	constexpr real_t default_z_precision = 0.0001;
	constexpr real_t default_ground_offset = 0.01;
	constexpr real_t default_spacing_precision = 0.00001;

	using RayTracer = HF::RayTracer::MultiRT; ///< Type of raytracer to be used internally.
	using pair = std::pair<int, int>; ///< Type for Directions to be stored as

	/*! \brief Cast an input value to real_t using static cast.
	
		\remarks Just a convience function. Same as static_cast<real_t>(t)

		\returns `static_cast<real_t>(t)`
	*/
	template<typename real_type>
	inline real_t CastToReal(real_type t) { return static_cast<real_t>(t); }
	
	/*! 
		\brief Cast an array of 3 values to the graph_generator's real_3 type

		\tparam real_3_type A vector type that is indexable with square brackets for up to 3 elements
						    (pointlike concept)
	
		\param t X,Y,Z coordinates to convert to a real3

		\returns  A real3 containing the x,y,z coordinates of `t`
	*/
	template<typename real_3_type>
	inline real3 CastToReal3(real_3_type t) {
		return real3{
			CastToReal(t[0]), CastToReal(t[1]), CastToReal(t[2])
		};
	};

	/*! \brief Various parameters to set the precision of certain parts of the graph generator. */
	struct Precision {
		real_t node_z;				///< Precision to round the z-component of nodes after a raycast is performed
		real_t node_spacing;		///< Precision to round nodes after spacing is calculated
		real_t ground_offset;		///< Distance to offset nodes from the ground before checking line of sight
	};
	/*!
	\brief Determines which geometry the ray will collide with.

	\remarks
	Used internally by the GraphGenerator to determine which type of geometry to collide with
	when casting a ray.
*/
	enum HIT_FLAG {
		/// No flag set.
		NO_FLAG = 0,
		/// Floors only.
		FLOORS = 1,
		/// Obstacles only.
		OBSTACLES = 2,
		/// Collide with floors and obstacles.
		BOTH = 3
	};

	using hashmap = std::unordered_map<int, HIT_FLAG>; ///< Hashmap used for assigning and storing hitflags for IDs
	
	/*! \brief Different rules for how geometry is filtered by the graph generator. 
	
		\remarks
		Each different filter mode was created to allow for different uses of the graph generator
		depending on how much geometry was specifically specified to be in either group. This
		allows for users to accept all geometry, Blacklist some geometry, or Blacklist and whitelist
		some geometry from graph generation. 
	*/
	enum class GeometryFilterMode {
		ALL_INTERSECTIONS = 0, ///< Consider all geometry as walkable surfaces
		OBSTACLES_ONLY = 1, ///< Consider all geometry that isn't an obstacle as walkable
		OBSTACLES_AND_FLOORS ///< Explicitly tag geometry ids as either obstacle or floor. Any ids outside of these ranges will always fail. 
	};

	/*! 
		\brief Manages rules and ids for different types of geometry in the graph generator.
	
		\see GeometryFilterMode for more info about each mode of this struct
		\see HIT_FLAG for info about the different geometry flags
		\see CheckGeometryID for using this struct to check the results of an intersection
	*/
	struct GeometryFlagMap {

	private:
		hashmap internal_dictionary;

		/*! \brief Set the filter mode of this GeometryFlagMap based on the input types. 
			
			\param walkable Array of walkable geometry IDs
			\param obstacle Array of obstacle geometry IDs
		
			\details
			If the walkable array is empty, then set the mode to obstacles only. 
			If the obstacle and the walkable arrays are empty set the mode to none
			If both obstacle and walkable arrays are specified, set this mode to 
			obstacles and floors.
			
			\post Sets the filter type for this hit_dict to the appropriate value for the number of elements
				  in walkable and obstacle
		*/
		inline void DetermineFilterMode(
			const std::vector<int>& walkable,
			const std::vector<int>& obstacle)
		{
			bool has_walkable = !(walkable.empty());
			bool has_obstacle = !(obstacle.empty());

			if (has_walkable && has_obstacle) this->Mode = GeometryFilterMode::OBSTACLES_AND_FLOORS;
			else if (has_obstacle && !has_walkable) this->Mode = GeometryFilterMode::OBSTACLES_ONLY;
			else this->Mode = GeometryFilterMode::ALL_INTERSECTIONS;
		}

	public:
		GeometryFilterMode Mode = GeometryFilterMode::ALL_INTERSECTIONS; ///< Filter mode of this GeometryFlagMap

		/*! \brief Set geometry ids as being walkable or obstacles.
		
			\param obstacle_geometry IDs of meshes to set as obstacles
			\param walkable_geometry IDs of meshes to set as walkable

			\post 1) The filter mode for this hit_dict will be set to the appropriate value for the inputs
			\post 2) All geometry in `obstacle_geometry` will return HIT_FLAG::OBSTACLES when calling operator []
					 and all geometry in `walkable_geometry` will return HIT_FLAG::FLOORS when calling operator[]
		*/
		inline void SetGeometryIds(
			const std::vector<int> & obstacle_geometry, 
			const std::vector<int> & walkable_geometry)
		{
			for (auto id : obstacle_geometry)
				this->Set(id, HIT_FLAG::OBSTACLES);
			for (auto id : walkable_geometry)
				this->Set(id, HIT_FLAG::FLOORS);

			DetermineFilterMode(walkable_geometry, obstacle_geometry);
		}

		/*! 
			\brief Check if this id exists in the dictionary.
			
			\param id ID to check for

			\returns True if the ID exists, false otherwise. 
		*/
		inline bool HasKey(int id) const {
			return internal_dictionary.count(id) >= 1;
		}

		/*!
			\brief Get the flag of the geometry in this hit dictionary. 
		
			\param id ID of geometry to get the hitflag of
			
			\returns
			The flag assigned to `id` in SetGeometryIDs or HIT_FLAG::NO_FLAG if no flag was ever
			assigned to `id`

			\remarks
			Unlike the standard square brackets operator of std::unordered_map, this gaurantees
			no memory is allocated. Note that this can't be used to assign keys. 
		*/
		inline HIT_FLAG operator[](int id) const {
			if (!HasKey(id)) return HIT_FLAG::NO_FLAG;
			return internal_dictionary.at(id);
		}

		/*! \brief Set the value of a key in the internal dictionary.
		
			\param id ID to set as 

			\post
			Next call to [] for this object with `id` will return `flag`. If `id` already has been
			assigned a flag, the value will be updated to `flag`.		
		*/
		inline void Set(int id, HIT_FLAG flag) {
			internal_dictionary[id] = flag;
		}
	};

	/*! \brief Holds parameters for the GraphGenerator. */
	struct GraphParams {
		real_t up_step;	 ///< Maximum height of a step the graph can traverse.Any steps higher this will be considered inaccessible.
		real_t up_slope; ///< Maximum upward slope the graph can traverse in degrees.Any slopes steeper than this will be considered inaccessible.
		real_t down_step; ///< Maximum step down the graph can traverse.Any steps steeper than this will be considered inaccessible.
		real_t down_slope; ///<	The maximum downward slope the graph can traverse. Any slopes steeper than this will be considered inaccessible.
		Precision precision; ///< Tolerances for the graph
		GeometryFlagMap geom_ids; ///< Stores a map of geometry IDs to their HIT_FLAGS and the current filter mode of the graph
	};

	/*! 
		\brief A simple wrapper for real3 that is able to determine whether or not it's defined.
		
		\details
		Upon construction, an optional_real_3 is put into 2 states, valid or invalid. Constructing
		this with the empty constructor will create an invalid optional_real3, using the x,y,z parameters
		will create a valid optional_real3. This can be checked by checking the truth the object similar
		to std::optional (i.e. if(optional_real3) ). 

		\remarks
		This is used in place of std::optional, since std::optional doesn't support arrays. In many ways
		it functions similarly, but doesn't feature the exact same syntax for the sake of simplicity. 
		This doesn't use any extra memory than a real3, and comes with the benefit of being able to 
		clearly indicate to callers that a function doesn't always return a usable value and 
		differentiate between code for checking a point and code for accessing a point in a single
		location.
	*/
	struct optional_real3 {
		real3 pt{ NAN, NAN, NAN }; ///< Point type.

		/*! \brief Construct an invalid optional_real3 */
		inline optional_real3() {};

		/*! 
			\brief Construct an  valid optreal3 from x,y,z parameters
			\param x X Coordinate
			\param y Y Coordinate
			\param z Z Coordinate
		*/
		inline optional_real3(real_t x, real_t y, real_t z) : pt(real3{ x,y,z }) {};
	
		/*! 
		\brief Construct an optreal3 from x,y,z parameters 
			\param in_real3 Input x,y,z coordinates in a real3
		*/
		inline optional_real3(const real3 & in_real3) : pt(in_real3) {};

		/*! \brief Get a reference to the point held by this optional_real3.
		
			\returns a reference to the point held by this optreal3. 
		*/
		inline real3 & operator *() { return pt; }
		
		/*!
			\brief Check if this optional_real_3 has a value.
			
			\returns True if this was initialized with x,y, and z coordinates that are not NAN, false otherwise.
		*/
		inline explicit operator bool() const {
			return !(std::isnan(pt[0]) && std::isnan(pt[1]) && std::isnan(pt[2]));
		}
	};



	/*! 
		\brief Calculate P(n,r) as an array with each unique permutaton of 2 values being a pair. 
		
		\param limit Highest number in the set

		\returns a list of all permutations for integers from 0 to limit
	*/
	std::set<std::pair<int, int>> permutations(int limit);

	/*! \brief Generate a graph of accessible space from a given start point.

		\details
		The Graph Generator maps out "accessible" space on a model from a given starting point. In graphs created
		by this algorithm, node represents a point in space that a human can occupy, and each edge between nodes 
		indicates that a human can traverse from one node to another node. The Graph Generator is a powerful tool 
		for analyzing space, since the graph or nodes it outputs can be used as input to all the analysis methods 
		offered by HumanFactors, allowing for it to be the starting point of other analysis methods within
		HumanFactors.
		
		\note 
		All arguments are in meters for distances and degrees for angles
		unless otherwise specified. For all calculations, the Graph Generator assumes
		geometry is Z-Up i.e. upstep is how high the step is in the z-direction, ground
		checks are performed in the -z direction etc. 
	*/
	class GraphGenerator
	{
	public:
		int max_nodes;			///< Maximum number of nodes to generate. If less than zero, generate nodes without a limit.
		int core_count;			///< Number of cores to use for graph generation.

		int max_step_connection; ///< Multiplier for number of children to generate. The higher this is, the more directions there will be
		real3 spacing;			///< Spacing between nodes. New nodes will be generated with atleast this much distance between them. 

		GraphParams params; ///< Parameters to run the graph generator. 

		RayTracer ray_tracer; ///< A pointer to the raytracer to use for ray intersections.
	public:
		
		/*! 
			\brief Construct a new graph generator with a specific raytracer.
			
			\param ray_tracer Raytracer to use for performing ray intersctions
			\param walkable_id IDs of geometry to be considered as obstacles 
			\param obstacle_id IDs of geometry to be considered as walkable surfaces
		
			\details
			Stores a pointer to RT. This could be dangerous in the case that the raytracer is destroyed before
			the graph generator is called again. It's not likely to occur in our codebase since GraphGenerators
			are mostly disposed of before this has a chance to become a problem. 
		*/
		GraphGenerator(HF::RayTracer::EmbreeRayTracer& ray_tracer, const std::vector<int> & obstacle_ids = std::vector<int>(0), const std::vector<int> & walkable_ids = std::vector<int>(0));
	
		/*!
			\brief Construct a new graph generator with a specific raytracer.

			\param ray_tracer Raytracer to use for performing ray intersctions
			\param walkable_id IDs of geometry to be considered as obstacles
			\param obstacle_id IDs of geometry to be considered as walkable surfaces

			\details
			Stores a pointer to RT. This could be dangerous in the case that the raytracer is destroyed before
			the graph generator is called again. It's not likely to occur in our codebase since GraphGenerators
			are mostly disposed of before this has a chance to become a problem.
		*/
		GraphGenerator(HF::RayTracer::NanoRTRayTracer& ray_tracer, const std::vector<int> & obstacle_ids = std::vector<int>(0), const std::vector<int>& walkable_ids = std::vector<int>(0));
		
		/*!
			\brief Construct a new graph generator with a specific raytracer.

			\param ray_tracer Raytracer to use for performing ray intersctions
			\param walkable_id IDs of geometry to be considered as obstacles
			\param obstacle_id IDs of geometry to be considered as walkable surfaces

			\details
			Stores a pointer to RT. This could be dangerous in the case that the raytracer is destroyed before
			the graph generator is called again. It's not likely to occur in our codebase since GraphGenerators
			are mostly disposed of before this has a chance to become a problem.
		*/
		GraphGenerator(HF::RayTracer::MultiRT& ray_tracer, const std::vector<int> & obstacle_ids = std::vector<int>(0), const std::vector<int>& walkable_ids = std::vector<int>(0));


		/*! 
			\brief Generate a graph of accessible space

			\param start_point The starting point for the graph generator. If this isn't above solid ground, no nodes will
			be generated.
			\param Spacing Space between nodes. Lower values will yield more nodes for a higher resolution graph.
			\param MaxNodes The maximum amount of nodes to generate.
			\param UpStep Maximum height of a step the graph can traverse. Any steps higher this will be considered inaccessible. 
			\param UpSlope Maximum upward slope the graph can traverse in degrees. Any slopes steeper than this
			 will be considered inaccessible.
			\param DownStep Maximum step down the graph can traverse. Any steps steeper than this will be considered inaccessible.

			\param DownSlope
			The maximum downward slope the graph can traverse. Any slopes steeper than this will be
			considered inaccessible.

			\param max_step_connections Multiplier for number of children to generate for each node. Increasing this value will
			 increase the number of edges in the graph, and as a result the amount of memory the
			 algorithm requires.

			\param cores Number of cores to use. -1 will use all available cores, and 0 will run a serialized version of the algorithm.
			 

			\param node_z_precision		  Precision to round the z-component of nodes after a raycast is performed
			\param node_spacing_precision Precision to round nodes after spacing is calculated
			\param ground_offset		  Distance to offset nodes from the ground before checking line of sight

			\returns The resulting graph or an empty graph if the start check failed
			 
			\note All parameters relating to distances are in meters, and all angles are in degrees.
			\note Geometry MUST be Z-UP in order for this to work. 
			
			\remarks
			This exists to provide a softer interface to the graph itself. Without this template, all callers would
			need to intuitively know the type of real_t or else their code wouldn't compile. The actual implementation
			of this function is in IMPL_BuildNetwork.

			The extensive use of templating on this function is to allow any numeric type to be used
			for any one of the template arguments. For example node_1_type could be a vector of floats
			while up_step_type could be an integer, and down_slope_type could be a float, etc. 
			
			\par Example
			\snippet tests\src\GraphGenerator.cpp EX_GraphGeneratorRayTracer
			\snippet tests\src\GraphGenerator.cpp EX_BuildNetwork
			\snippet tests\src\GraphGenerator.cpp EX_PrintGraph
			`[(0, 0, -0),(-1, -1, -0),(-1, 0, -0),(-1, 1, 0),(0, -1, -0),(0, 1, 0),(1, -1, -0),(1, 0, -0),(1, 1, 0),(-2, -2, -0),(-2, -1, -0),(-2, 0, -0),(-1, -2, -0),(0, -2, -0),(-2, 1, 0),(-2, 2, 0),(-1, 2, 0),(0, 2, 0),(1, -2, -0)]`
		*/
		template <
			typename node_type,
			typename node2_type,
			typename up_step_type,
			typename up_slope_type,
			typename down_step_type,
			typename down_slope_type,
			typename z_precision_type =  real_t,
			typename connect_offset_type = real_t,
			typename spacing_precision_type = real_t
		>
		inline SpatialStructures::Graph BuildNetwork(
			const node_type& start_point,
			const node2_type& Spacing,
			int MaxNodes,
			up_step_type UpStep,
			up_slope_type UpSlope,
			down_step_type DownStep,
			down_slope_type DownSlope,
			int max_step_connections,
			int cores = -1,
			z_precision_type node_z_precision = default_z_precision,
			connect_offset_type  node_spacing_precision = default_spacing_precision,
			spacing_precision_type ground_offset = default_ground_offset
		) {
			assert(node_z_precision != 0);
			return IMPL_BuildNetwork(
				CastToReal3(start_point),
				CastToReal3(Spacing),
				MaxNodes,
				CastToReal(UpStep),
				CastToReal(UpSlope),
				CastToReal(DownStep),
				CastToReal(DownSlope),
				max_step_connections,
				cores,
				CastToReal(node_z_precision),
				CastToReal(node_spacing_precision),
				CastToReal(ground_offset)
			);
		}


		/*!
			\brief Generate a graph of accessible space

			\param start_point The starting point for the graph generator. If this isn't above solid ground, no nodes will
			be generated.
			\param Spacing Space between nodes. Lower values will yield more nodes for a higher resolution graph.
			\param MaxNodes The maximum amount of nodes to generate.
			\param UpStep Maximum height of a step the graph can traverse. Any steps higher this will be considered inaccessible. 
			\param UpSlope Maximum upward slope the graph can traverse in degrees. Any slopes steeper than this
			 will be considered inaccessible.
			\param DownStep Maximum step down the graph can traverse. Any steps steeper than this will be considered inaccessible.

			\param DownSlope
			The maximum downward slope the graph can traverse. Any slopes steeper than this will be
			considered inaccessible.

			\param max_step_connections Multiplier for number of children to generate for each node. Increasing this value will
			 increase the number of edges in the graph, and as a result the amount of memory the
			 algorithm requires.

			\param cores Number of cores to use. -1 will use all available cores, and 0 will run a serialized version of the algorithm.
			 

			\param node_z_precision		  Precision to round the z-component of nodes after a raycast is performed
			\param node_spacing_precision Precision to round nodes after spacing is calculated
			\param ground_offset		  Distance to offset nodes from the ground before checking line of sight

			\returns The resulting graph or an empty graph if the start check failed.
			 
			\note All parameters relating to distances are in meters, and all angles are in degrees.
			\note Geometry MUST be Z-UP in order for this to work. 
			
		
		*/
		SpatialStructures::Graph IMPL_BuildNetwork(
			const real3& start_point,
			const real3& Spacing,
			int MaxNodes,
			real_t UpStep,
			real_t UpSlope,
			real_t DownStep,
			real_t DownSlope,
			int max_step_connections,
			int cores = -1,
			real_t node_z_precision = default_z_precision,
			real_t node_spacing_precision = default_spacing_precision,
			real_t ground_offset = default_ground_offset
		);


		/*!
			\brief Perform breadth first search to populate the graph with with nodes and edges. 

			\param todo Todo list to hold unchecked nodes. Must atleast contain a single start point.

			\pre todo contains the starting point for the graph.

			\see CrawlGeomParallel for a parallel version.

			\returns The Graph generated by performing the breadth first search. 

			\par Example
			\snippet tests\src\GraphGenerator.cpp EX_GraphGeneratorRayTracer
			\snippet tests\src\GraphGenerator.cpp EX_CrawlGeom
			\snippet tests\src\GraphGenerator.cpp EX_CrawlGeom_Serial
			\snippet tests\src\GraphGenerator.cpp EX_PrintGraph
			`[(0, 1, 0),(-1, 0, -0),(-1, 1, 0),(-1, 2, 0),(0, 0, -0),(0, 2, 0),(1, 0, -0),(1, 1, 0),(1, 2, 0),(-2, -1, -0),(-2, 0, -0),(-2, 1, 0),(-1, -1, -0),(0, -1, -0),(-2, 2, 0),(-2, 3, 0),(-1, 3, 0),(0, 3, 0),(1, -1, -0)]`

		*/
		SpatialStructures::Graph CrawlGeom(UniqueQueue& todo);
		
		/*!
			\brief Perform breadth first search to populate the graph with nodes and edges using multiple cores.

			\param todo Todo list to hold unchecked nodes. Must atleast contain a single start point.

			\pre todo contains the starting point for the graph.

			\returns The Graph generated by performing the breadth first search.

			\par Example
			\snippet tests\src\GraphGenerator.cpp EX_GraphGeneratorRayTracer
			\snippet tests\src\GraphGenerator.cpp EX_CrawlGeom
			\snippet tests\src\GraphGenerator.cpp EX_CrawlGeom_Parallel
			\snippet tests\src\GraphGenerator.cpp EX_PrintGraph

			`[(0, 2, 0),(-1, 1, -0),(-1, 2, 0),(-1, 3, 0),(0, 1, -0),(0, 3, 0),(1, 1, -0),(1, 2, 0),(1, 3, 0),(1, 0, -0),(0, -1, -0),(0, 0, -0),(1, -1, -0),(2, -1, -0),(2, 0, -0),(2, 1, -0),(2, 2, 0),(2, 3, 0),(-2, -1, -0),(-3, -2, -0),(-3, -1, -0),(-3, 0, -0),(-2, -2, -0),(-2, 0, -0),(-1, -2, -0),(-1, -1, -0),(-1, 0, -0)]`
		*/
		SpatialStructures::Graph CrawlGeomParallel(UniqueQueue& todo);
	};

	/*! 
		\brief Determine if the start point of the graph is over valid ground. 
	
		\param RT Raytracer to use for ray intersection
		\param start_point the x,y,z coordinates of the starting point
		\param Params parameters to use for precision.

		\returns An invalid optional_real3 if the check failed, or a valid optional_real3 containing
				 the x,y,z coordinates of the start point moved to be directly ontop of the intersected ground
				 if the check succeeded.

		\par Example
		\snippet tests\src\GraphGenerator.cpp EX_GraphGeneratorRayTracer
		\snippet tests\src\GraphGenerator.cpp EX_ValidateStartPoint
		`(0, 0, 0)`
	
	*/
	optional_real3 ValidateStartPoint(RayTracer& RT, const real3& start_point, const GraphParams & Params);
	
	/*!
		\brief Cast a ray and get the point of intersection if it connects.

		\param RT Raytracer to use for intersection.
		\param origin Origin point of the ray.
		\param direction direction to cast the ray in.
		\param node_z_tolerance Precision to round the point of intersection's z-component to
		\param flag Which category of geometry will be intersected with. In general, any intersections with
					geometry other than this type will be discarded unless the type is BOTH, NONE, or the
					geometry dictionary is empty. 
		\param geometry_dict Dictionary containing rules for filtering ray intersections. If not specified,
							 this will not filter any intersections.

		\returns An invalid optional_real3 if the ray did not intersect any geometry, or a valid 
				 optional_real3 containing the point of intesection if an intersection was found.

		\par Example
		\snippet tests\src\GraphGenerator.cpp EX_GraphGeneratorRayTracer
		\snippet tests\src\GraphGenerator.cpp EX_CheckRay

		`(1, 1, 0)`
	*/
	optional_real3 CheckRay(
		RayTracer& RT,
		const real3& origin,
		const real3& direction,
		real_t node_z_tolerance,
		HIT_FLAG flag = BOTH,
		const GeometryFlagMap& geometry_dict = GeometryFlagMap()
	);


	/*! 
		\brief Create a set of directions based on max_step_connections
	
		\param max_step_conenctions Multiplier for directions to generate.

		\returns A set of all directions generated for max_step_connections.

		\see permutations for more information on how max_step_connections 
			 influences generated directions

		\par Example
		\snippet tests\src\GraphGenerator.cpp EX_CreateDirecs
		`[(-1, -1),(-1, 0),(-1, 1),(0, -1),(0, 1),(1, -1),(1, 0),(1, 1),(-2, -1),(-2, 1),(-1, -2),(-1, 2),(1, -2),(1, 2),(2, -1),(2, 1)]`
	*/
	std::vector<pair> CreateDirecs(int max_step_connections);

	/*! 
		\brief Calculate all possible edges between parent and possible_children
		
		\param parent Parent of all children in `possible_children`
		\param possible_children Children that may have an edge with Parent
		\param rt Raytracer to use for ray intersections
		\param GP parameters to use for rounding and discarding nodes

		\par Rules
		An edge is considered valid if:
		1) Both parent and child the potential child are over valid walkable ground
		2) After the child is moved to be directly on top of the ground it is over
		   The slope between parent and child is within upslope/downslope limits OR
		   the path between parent/child involves a step that is within upstep/downstep limits
		
		\returns An array of all edges that could be created between parent and possible_children 
				 that meet the requirements of GraphParameter. For every edge, the child will be 
				 moved downwards to ensure it's over valid ground. 


		\par Example
		\snippet tests\src\GraphGenerator.cpp EX_GraphGeneratorRayTracer
		\snippet tests\src\GraphGenerator.cpp EX_CreateChildren
		`[((0, 2, 0), 2.23607, 1),((2, 0, -0), 2.23607, 1)]`
	*/
	std::vector<graph_edge> GetChildren(
		const real3 & parent,
		const std::vector<real3>& possible_children,
		RayTracer  & rt,
		const GraphParams & GP
	);

	/*! 
		\brief Populare out_children with a potential child position for every direction in directions
		
		\param parent Parent to generate children from.
		\param directions X,Y pairs to use for generating children. I.E. A pair of `{1,2}`would create a child
						  at `{parent.x + 1*spacing.x, parent.y + 2*spacing.y, parent.z + spacing.z}.`
		\param spacing Magnitude to offset children in the x,y,z direction for each direction in directions
		\param gp Parameters to use for rounding offset nodes. 

		\returns An array of children offset from parent by spacing and directions. 

		\par Example

		\par Example
		\snippet tests\src\GraphGenerator.cpp EX_CreatePotentialChildren
		`[(0, 2, 4),(0, 4, 4),(1, 0, 4),(2, 0, 4),(1, 2, 4),(2, 2, 4)]`
	
	*/
	std::vector<real3> GeneratePotentialChildren(
		const real3& parent,
		const std::vector<pair>& direcs,
		const real3& spacing,
		const GraphParams & gp
	);

	/*!
		\brief Move a node in `direction` by `dist` units.

		\tparam D Must have [0], [1], [2] defined.
		\tparam N Must have [0], [1], [2] defined.

		\param dist Distance to move the node.
		\param direction Direction to move the node in.
		\param node Node to move

		\post Node's members at [0], [1], [2] been moved in `direction` by `dist` units.
	*/
	template<typename A, typename D, typename N>
	inline void MoveNode(const A& dist, const D& direction, N& node) {
		node[0] = std::fma(direction[0], dist, node[0]);
		node[1] = std::fma(direction[1], dist, node[1]);
		node[2] = std::fma(direction[2], dist, node[2]);
	}

	/*! \brief Determine whether children are over valid ground, and and meet upstep/downstep requirements

		\param parent Parent of all children in possible_children
		\param possible_children Children of parent that may or may not be over valid ground
		\param rt Raytracer to use for all ray intersections
		\param params Parameters to use for upstep/downstep limits and rounding

		\returns An array of children from `possible_children` that are over valid ground and meet the
				 upstep/downstep requirements in `params`. Any child that didn't meet these requirements
				 will not have been included. Each child in the return will have been moved to be directly
				 over the valid ground they're above. 

		
		\par Example
		\snippet tests\src\GraphGenerator.cpp EX_GraphGeneratorRayTracer
		\snippet tests\src\GraphGenerator.cpp EX_CheckChildren
		`[(0, 2, 0),(1, 0, -0),(0, 1, 0),(2, 0, -0)]`
	*/
	std::vector<real3> CheckChildren(
		const real3& parent,
		const std::vector<real3>& possible_children,
		RayTracer& rt,
		const GraphParams & params
	);

	/*! 
		\brief Determine what kind of step (if any) is between parent and child.
	
		\param parent Node being traversed from
		\param child  Node being traversed to
		\param rt Raytracer to use for all ray intersections
		\param params Parameters to use for upstep/downstep and upslope/downslope

		\returns The type of step between parent/child or NO_CONNECTION if no connection
				 could be found between them. 

		\par Example
		\snippet tests\src\GraphGenerator.cpp EX_GraphGeneratorRayTracer
		\snippet tests\src\GraphGenerator.cpp EX_CheckConnection
		`[1,0,0,1]`
	*/
	HF::SpatialStructures::STEP CheckConnection(
		const real3 & parent,
		const real3 & child,
		RayTracer& rt,
		const GraphParams & params
	);

	/*! 
		\brief Determine if there is a valid line of sight between parent and child
		
		\param parent Node to perform the line of sight check from
		\param child  Node to perform the line of sight check to
		\param RT raytracer to use for the line of sight check

		\returns `True` The line of sight between parent and child is obstructed
		\returns `False` There is a clear line of sight between parent and child
		
		\par Example

		\snippet tests\src\GraphGenerator.cpp EX_GraphGeneratorRayTracer
		\snippet tests\src\GraphGenerator.cpp EX_CheckOcclusion
		`Occlusion Check For Child 1 = True`\n
		`Occlusion Check For Child 2 = False`
	*/
	bool OcclusionCheck(const real3 & parent, const real3 & child, RayTracer& RT);

	/*!
		\brief Determine if the slope between parent and child is traversable according to the graph parameters.

		\param parent Node being traversed from
		\param child Node being traversed to
		\param GP Parameters to use for up_slope/down_slope

		\returns `true` if the slope from parent to child meets upstep/downstep requirements in graph_params.
				 `false` otherwise.
		\par Example
		\snippet tests\src\GraphGenerator.cpp EX_CheckSlope

		`Slope Check For Child 1 = True`\n
		`Slope Check For Child 2 = False`
	*/
	bool CheckSlope(
		const real3 & parent,
		const real3 & child,
		const GraphParams & gp
	);

}

