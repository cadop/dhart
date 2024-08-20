///
/// \file		view_analysis.h
/// \brief		Contains definitions for the <see cref="HF::ViewAnalysis">ViewAnalysis</see> namespace
///
///	\author		TBA
///	\date		26 Jun 2020

#include <vector>
#include <array>
#include <cmath>
#include <iostream>
#include <assert.h>
#include <limits>

#ifndef VIEW_ANALYSIS_G
#define VIEW_ANALYSIS_G

// Forward Declares
namespace HF {
	namespace SpatialStructures {
		struct Node;
	}
	namespace RayTracer {
		class EmbreeRayTracer;
		struct RayRequest;
	}
}

/*! 

	\brief Analyze space from the perspective of observers within a 3D environment.

	\details 
	View Analysis contains a set of algorithms dedicated to evaluating the view of an observer
	from specific points in a model. Generally this consists of equally distributing a series
	of rays in a sphere around the observer, then casting the rays and calculating a result or
	returning the raw results of each intersection.

	\see FibbonacciDistributePoints for the algorithm used to equally distribute rays.
	\see SphericalViewAnalysis for a view analysis algorithm that returns the ID and distance to 
	intersection point of every ray casted.
	\see SphericalRayshootWithAnyRTForDistance for an algorithm that aggregates the results of
	view analysis into a score for each of the input observer points.
*/
namespace HF::ViewAnalysis {
	/// <summary>
	/// The type of aggregation to use for <see cref="ViewAnalysisAggregate"/>
	/// </summary>
	enum class AGGREGATE_TYPE {
		/// Total number of intersections.
		COUNT = 0,
		/// Sum of the distance from the origin to all intersections.
		SUM = 1,
		/// Average distance from the origin to every intersection.
		AVERAGE = 2,
		/// Maximum distance from the origin to any intersection.
		MAX = 3,
		/// Minimum distance from the origin to any intersection
		MIN = 4
	};

	/// <summary> Evenly distribute a set of points around a sphere centered at the origin. </summary>
	/// <param name="num_points"> Maximum number of points to distribute. </param>
	/// \param upward_limit Maximum angle in degrees to cast rays above the viewpoint.
	/// \param downward_limit Maximum angle in degrees to cast rays below the viewpoint.
	
	/// \returns A vector of arrays containing the x, y, and z coordinates of each point on the sphere.
	/*!  
		\ingroup ViewAnalysis
		\note
		The number of points returned by this function will not exactly equal the amount of points
		specified by num_points depending on the values of upwards_fov and downward fov. More information is available below.
		\details
		\par How FOV is implemented
		Every point is equally distributed on a sphere using a formula based on this stack overflow answer:
		https://stackoverflow.com/questions/9600801/evenly-distributing-n-points-on-a-sphere. This formula
		cannot easily be mapped to a specific vertical fov or cone, so instead this function will remove points
		that don't meet the constraints. Discarding points causes the number of points	generated to be less than
		the num_rays argument, so the percentage of points discarded is calculated and a second run is performed
		with an adjusted num_rays value to get as close as possible to the	value specified in the argument.
		This approach will not result in an exact match to the num_rays	argument, and will run
		\link FibbonacciDist \endlink twice.

		\par Example
		\code
			// Requires #include "view_analysis.h"

			// For brevity
			using HF::ViewAnalysis::FibbonacciDistributePoints;

			// Prepare maximum number of points to distribute
			int size = 8;

			// Generate points without any limitations on fov
			auto points = FibbonacciDistributePoints(size, 90.0f, 90.0f);

			// Print number of points
			std::cout << "Number of Points:" << points.size() << std::endl;

			// Iterate through results and print every point.
			std::cout << "[";
			for (int i = 0; i < size; i++) {
				const auto& point =  points[i];
				std::cout << "(" << point[0] << ", " << point[1] << ", " << point[2] << ")";
				if (i != size - 1) std::cout << ", ";
			}
			std::cout << "]" << std::endl;
		\endcode


		`>>> Number of Points:8`\n
		`>>> [(-0, -1, 0), (-0.304862, -0.75, -0.586992), (0.813476, -0.5, 0.29708), (-0.894994, -0.25, 0.369441),`
			`(0.423846, 0, -0.905734), (0.289781, 0.25, 0.923865), (-0.749296, 0.5, -0.43423), (0.64601, 0.75, -0.142025)]`
	
	*/
	std::vector<std::array<float, 3>> FibbonacciDistributePoints(
		int num_points,
		float upwards_fov = 50.0f,
		float downward_fov = 70.0f
	);
	/// </summary>
	/// Apply an aggregation to the given value.
	/// </summary>
	/// <param name="out_total">The total of the current aggregation. This will be modified depending on agg_type.</param>
	/// <param name="new_value"> New value to aggregate into out_total. </param>
	/// <param name="agg_type">The type of aggregation to use.</param>
	/// <param name="count">Number of elements encountered so far. Used for Count/Average.</param>
	/// <exception cref="std::out_of_range">agg_type didn't match any valid AGGREGATE_TYPE.</exception>
	/*!
		\details This can be called in a loop to summarize the results of some calculation as new values
		become available. This avoids having to allocate entire arrays of values then calculating the result
		at the end.	\see AGGREGATE_TYPE for a list of supported aggregation types.

		\note This function will likely not be very useful elsewhere since most of its functionality
		can be replaced by single function calls like std::min(a, b) or std::max(a,b). This was made a standalone function
		mostly to seperate calculating the result value from the implementation of the View	Analysis itself.

		\par Example
		\code
			// Requires #include "view_analysis.h", #include <numeric>

			// Use this to save some space.
			using HF::ViewAnalysis::AGGREGATE_TYPE;
			using HF::ViewAnalysis::Aggregate;

			// Undef these since they will prevent us from calling numericlimits
			#undef min
			#undef max

			// Define values
			std::vector<int> values = { 1, 2, 3, 4, 5 };

			// Calculate Average
			float total = 0.0f;	int count = 0;
			for (int val : values) {
				count += 1;
				Aggregate(total, val, AGGREGATE_TYPE::AVERAGE, count);
			}
			std::cerr << "Average: " << total << std::endl;

			// Calculate Sum
			total = 0.0f;
			for (int val : values) Aggregate(total, val, AGGREGATE_TYPE::SUM);
			std::cerr << "Sum: " << total << std::endl;

			// Calculate Max. Start total at lowest possible float value to ensure
			// it overwritten by the first element.
			total = std::numeric_limits<float>::min();
			for (int val : values) Aggregate(total, val, AGGREGATE_TYPE::MAX);
			std::cerr << "Max: " << total << std::endl;

			// Calculate Min. Start total at highest possible float value to ensure
			// it overwritten by the first element.
			total = (std::numeric_limits<float>::max());
			for (int val : values) Aggregate(total, val, AGGREGATE_TYPE::MIN);
			std::cerr << "Min: " << total << std::endl;

			// Calculate Count
			total = 0.0f;
			for (int val : values) Aggregate(total, val, AGGREGATE_TYPE::COUNT);
			std::cerr << "Count: " << total << std::endl;
		\endcode

		`>>> Average: 3`\n
		`>>> Sum: 15`\n
		`>>> Max: 5`\n
		`>>> Min: 1`\n
		`>>> Count: 5`\n

		\todo Min/Max and maybe the other functions should handle NaNs for min/max to signal that no value already exists.
		 The user can use the min and max numeric limits or zero for defaults like in the example, but it would be great to
		 be able to use one value that would be reliably handled as default input for every case.
		*/
	inline void Aggregate(float& out_total, float new_value, const AGGREGATE_TYPE agg_type, int count = 0) {
		switch (agg_type) {
			case AGGREGATE_TYPE::COUNT:
				if (new_value > 0) out_total += 1;
				break;
			case AGGREGATE_TYPE::SUM:
				out_total += new_value;
				break;
			case AGGREGATE_TYPE::AVERAGE: {
				int n = count - 1;
				out_total = (n * (out_total)+new_value) / count;
				break;
			}
			case AGGREGATE_TYPE::MAX: {
				out_total = std::max<float>(out_total, new_value);
				break;
			}
			case AGGREGATE_TYPE::MIN: {
				out_total = std::min<float>(out_total, new_value);
				break;
			}
			default:
				throw std::out_of_range("Unimplemented aggregation type");
				break;
		} // End Switch
		assert(out_total == 0 || std::isnormal(out_total));
		return;
	}

	/// Conduct view analysis with any Raytracer in parallel.
	/// </summary>
	/// <param name="ray_tracer"> A valid raytracer that already has the geometry loaded. </param>
	/// <param name="Nodes"> Points to perform analysis from. </param>
	/// <param name="num_rays"> The number of rays to cast from each point in nodes. The actual amount of rays cast
	/// may be less or more than this number. Due to how the spherical ray distribution is calculated. </param>
	/// \param upward_limit Maximum angle in degrees to cast rays above the viewpoint.
	/// \param downward_limit Maximum angle in degrees to cast rays below the viewpoint.
	/// \param height Height off the ground to cast from. All points in Nodes will be offset
	/// this distance from the ground (+Z) before calculations are performed
	/*!
		\ingroup ViewAnalysis
		\tparam RES A class or struct that has a .SetHit() function. This function will be called with the
		node, direction, distance to intersection, and MeshID intersected for every ray that intersects
		geometry.

		\tparam RT A Raytracer with IntersectOutputArguments defined for the type of N.
		\tparam N A point that overloads [] for 0, 1 and 2.

		\return
		An \a approximately num_rays * Nodes.size() long vector of RES with one element for every ray cast.
		Results will be laid out in order with the first node's results first, then the second, etc. Every ray that
		intersected something, and all elements that didn't result in an intersection will left at their default values.

		\details
		ViewAnalysis is calculated by casting a series of rays equally distributed in a sphere from each point in Nodes.
		This function will run in parallel using all available cores. Depending on RES, this function's complexity and results
		can vary.

		\exception std::bad_array_new_length The number of rays is larger than that which can be stored
		in a std::vector.

		\see FibbonacciDistributePoints For details on how the directions are calculated from num_rays.
		\see SphericalRayshootWithAnyRTForDistance for a more efficent method of getting a summary of the results.

		\par Example
		\code
			// Requires #include "view_analysis.h"

			// Use this so we can fit within 80 characters
			using HF::ViewAnalysis::SphericalViewAnalysis;
			using HF::RayTracer::EmbreeRayTracer;
			using HF::Geometry::MeshInfo;

			// Create Plane
			const std::vector<float> plane_vertices{
				-10.0f, 10.0f, 0.0f,
				-10.0f, -10.0f, 0.0f,
				10.0f, 10.0f, 0.0f,
				10.0f, -10.0f, 0.0f,
			};

			const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

			// Create RayTracer
			EmbreeRayTracer ert(std::vector<MeshInfo>{
				MeshInfo(plane_vertices, plane_indices, 0, " ")}
			);

			// Define a struct that only stores the distance when sethit is called.
			struct SampleResults {
				float dist = -1.0f;

				inline void SetHit(
					const std::array<float, 3> & node,
					const std::array<float, 3> & direction,
					float distance,
					int meshID
				) {
					dist = distance;
				}
			};

			// Define observer points
			std::vector<std::array<float, 3>> points{
				{0,0,0}, {1,1,0}, {1,2,0}, {1000, 1000, 0}
			};

			// Perform View Analysis
			int num_rays = 50;
			auto results = SphericalViewAnalysis<SampleResults>(ert, points, num_rays);

			// Determine how many directions there were since there may have been less than
			// what we specified
			int num_directions = results.size() / points.size();

			// Construct a vector from the results of the first node
			std::vector<SampleResults> first_results(
				results.begin(), results.begin() + num_directions
			);

			// Print Results
			std::cerr << "(";
			for (int i = 0; i < first_results.size(); i++) {
				const auto & result = first_results[i];
				std::cerr << result.dist;

				if (i != first_results.size() - 1) std::cerr << ", ";
			}
			std::cerr << ")" << std::endl;
		\endcode


		`>>> (-1, 7.35812, -1, -1, 3.70356, -1, 5.56647, 12.1517, -1, 2.36725, -1, -1, 2.97477, 2.58713, -1,`
		`	-1, 1.91404, 5.95885, 4.26368, 1.86167, -1, -1, 2.0406, 2.78304, -1, -1, -1, -1, 2.83909, 2.05302, `
		`	-1, -1, 1.90724, 4.29017, 6.3381, 1.98544, -1,	-1, 2.75554, 3.15929, -1, -1, 2.6345, -1, -1,`
		`	6.80486, -1, 5.12012, -1)`

		\todo Should this have a height check like the one in the VisibilityGraph?

	*/
	template <typename RES, typename RT, typename N>
	std::vector<RES> SphericalViewAnalysis(
		RT& ray_tracer,
		const std::vector<N>& Nodes,
		int num_rays,
		float upward_limit = 50.0f,
		float downward_limit = 70.0f,
		float height = 1.7f)
	{
		// Calculate directions then perform a quick check to see if we can even hold this vector
		const auto directions = FibbonacciDistributePoints(num_rays, upward_limit, downward_limit);
		int required_vector_size = directions.size() * Nodes.size();
		std::vector<RES> out_results;

		// Throw if we can't fit the results into a single vector. This will change based on the implementation
		// of std::vector being used.
		if (out_results.max_size() < required_vector_size) {
			std::cout << "The desired view analysis setting exceed the maximum vector size!" << std::endl;
			throw std::bad_array_new_length();
		}
		out_results.resize(required_vector_size);

		// Size of the vector is the number of rays
		num_rays = directions.size();

		#pragma omp parallel
		{
		// Start parallel intersections
		#pragma omp for schedule(dynamic) 
			for (int i = 0; i < Nodes.size(); i++)
			{
				// Reset out values to temporarily store results. Kept private by defining here
				float out_distance = 0;
				int out_mid = 0;

				auto node = Nodes[i];
				node[2] += height;
				int os = directions.size() * i;

				// Iterate through every direction and cast a ray for it
				for (int k = 0; k < directions.size(); k++)
				{
					int idx = os + k;
					// Call the result's SetHit if it intersected.
					if (ray_tracer.IntersectOutputArguments(node, directions[k], out_distance, out_mid))
					{
						out_results[idx].SetHit(node, directions[k], out_distance, out_mid);
					}
				} // End direction loop
			} // End node loop
		} // End omp space

		return out_results;
	}

// To use numeric limits, need to undefine max
#undef max

	/// <summary> Conduct view analysis and recieve a summarized set of results for each node. </summary>
	/// <param name="ray_tracer"> A valid raytracer that already has the geometry loaded. </param>
	/// <param name="Nodes"> Points to perform analysis from. </param>
	/// <param name="num_rays">
	/// The number of rays to cast from each point in nodes. The actual amount of rays cast may be
	/// less or more than this number. Due to how the spherical ray distribution is calculated. 
	/// </param>
	/*!
		\ingroup ViewAnalysis
		\param upward_limit Maximum angle in degrees to cast rays above the viewpoint.
		\param downward_limit Maximum angle in degrees to cast rays below the viewpoint.
		\param height Height off the ground to cast from. All points in Nodes will be offset
		this distance from the ground (+Z) before calculations are performed
		\param aggregation The type of aggregation to use.

		\tparam RT A Raytracer with IntersectOutputArguments defined for the type of N.
		\tparam N A point that overloads [] for 0, 1 and 2.

		\return
		A Nodes.size() long array of results for each node in Nodes.

		\details
		The analysis performed by this function is identical to that described in SphericalViewAnalysis, but
		results are summarized over the course of the analysis for every observer point instead of recording
		each individual intersection. The memory usage for this function is drastically lower than that
		of SphericalViewAnalysis, resulting in lower execution times at the cost of being restricted to a set
		of predefined aggregation methods.

		\exception std::bad_array_new_length The number of rays is larger than that which can be stored
		in a std::vector.

		\see FibbonacciDistributePoints For details on how the directions are calculated from num_rays.
		\see SphericalViewAnalysis to get the result of every ray cast instead of summarizing the results.
		\see AGGREGATE_TYPE for a list of aggregation methods.
		\see Aggregate for the implementation of all aggregation methods.

		\par Example
		\code
			// Requires #include "view_analysis.h"

			// Use this so we can fit within 80 characters
			using HF::ViewAnalysis::SphericalRayshootWithAnyRTForDistance;
			using HF::ViewAnalysis::AGGREGATE_TYPE;
			using HF::RayTracer::EmbreeRayTracer;
			using HF::Geometry::MeshInfo;

			// Create Plane
			const std::vector<float> plane_vertices{
				-10.0f, 10.0f, 0.0f,
				-10.0f, -10.0f, 0.0f,
				10.0f, 10.0f, 0.0f,
				10.0f, -10.0f, 0.0f,
			};
			const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

			// Create RayTracer
			EmbreeRayTracer ert(std::vector<MeshInfo>{
				MeshInfo(plane_vertices, plane_indices, 0, " ")}
			);

			// Define observer points
			std::vector<std::array<float, 3>> points{
				{0,0,0}, {10,10,0}, {20,20,0}, {30, 30, 0}
			};

			// Perform View Analysis and sum the distance to all intersections
			// for every node
			int num_rays = 2000;
			auto results = SphericalRayshootWithAnyRTForDistance(
				ert, points, num_rays, 90.0f, 90.0f, 1.7f, AGGREGATE_TYPE::SUM
			);

			// Print Results
			std::cerr << "(";
			for (int i = 0; i < results.size(); i++) {
				std::cerr << results[i];

				if (i != results.size() - 1) std::cerr << ", ";
			}
			std::cerr << ")" << std::endl;
		\endcode

		`>>> (2746.72, 932.565, 170.858, 76.8413)`

		\todo Should this have a height check like the one in the VisibilityGraph?

		\todo Rename this to ViewAnalysisAggregate.

		\todo Potential speed up here by using a Raytracer function that doesn't return the point of
		intersection.
	*/
	template<typename RT, typename N>
	std::vector<float> SphericalRayshootWithAnyRTForDistance(
		RT& ray_tracer,
		const std::vector<N>& Nodes,
		int num_rays,
		float upward_limit = 50.0f,
		float downward_limit = 70.0f,
		float height = 1.7f,
		const AGGREGATE_TYPE aggregation = AGGREGATE_TYPE::SUM)
	{
		// Allocate score array and calculate directions
		std::vector<float> out_scores(Nodes.size());
		const auto directions = FibbonacciDistributePoints(num_rays, upward_limit, downward_limit);

		#pragma omp parallel
		{
			// Start parallel intersections
		#pragma omp for schedule(dynamic) 
			for (int i = 0; i < Nodes.size(); i++)
			{
				const auto& node = Nodes[i];

				// Get a reference to the value in the score array, reset score and count to zero
				float& score = out_scores[i];
				int count = 1;

				// If the aggregation type is min, set the score to some large value
				if (aggregation == AGGREGATE_TYPE::MIN){
					score = std::numeric_limits<float>::max();
				}
				else {
					score = 0;
				}

				// Cast a ray for every direction in directions
				for (const auto& direction : directions)
				{
					// Create a copy of the node to offset.
					auto node_copy = node;
					node_copy[2] += height;
					//float distance = 0;

					// If the ray intersects any geometry, calculate distance to the intersection point
					// then plug that into the aggregation method.
					if (ray_tracer.PointIntersection(
							node_copy[0], node_copy[1], node_copy[2],
							direction[0], direction[1], direction[2]))
					{
						// Add the height back to the original node so the distance check is accurate
						float distance = sqrtf(
												  powf((node[0]		   - node_copy[0]), 2) 
												+ powf((node[1]		   - node_copy[1]), 2) 
												+ powf((node[2] + height - node_copy[2]), 2));

						// Based on aggregation method, check this new value against the last
						Aggregate(score, distance, aggregation, count);
						count++;
					}
				}// End FOR over directions
			} // End FOR over Nodes
		}// End OMP
		return out_scores;
	}
}

#endif
