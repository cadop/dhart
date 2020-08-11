/*!
	\file		view_analysis_C.h
	\brief		Header file for conducting view analysis via the C Interface

	\author		TBA
	\date		11 Aug 2020
*/

#include <cinterface_utils.h>
#include <vector>
#include <raytracer_C.h>

#define C_INTERFACE extern "C" __declspec(dllexport) int

namespace HF {
	namespace SpatialStructures {
		struct Node;
		class Graph;
	}
}

/*!
	\enum		AGGREGATE_TYPE
	\brief		Determines how to aggregate edges from the results of view analysis
*/
enum class AGGREGATE_TYPE {
	COUNT = 0,		///< Number of rays that hit.
	SUM = 1,		///< Sum of distances from the origin to each of its hit points.
	AVERAGE = 2,	///< Average distance of origin to its hit points.
	MAX = 3,		///< Maximum distance from origin to its hit points.
	MIN = 4			///< Minimum distance from origin to its hit points.
};

/*!
	\defgroup		ViewAnalysis
	Analyze the view from points in the environment.

	@{
*/

/*!
	\brief	Conduct view analysis, then aggregate the results

	\param	ERT				Raytracer containing the geometry to use for ray intersections.
	\param	node_ptr		Observer points for the view analysis.
	\param	node_size		Number of nodes in the array pointed to by node_ptr.

	\param	max_rays		Number of rays to cast for each node in node_ptr. 
							Note that this may fire fewer rays than max_rays, depending on FOV restrictions.

	\param	upward_fov		Maximum degrees upward from the viewer's eye level to consider.
	\param	downward_fov	Maximum degrees downward from the viewer's eye level to consider. 
	\param	height			Height to offset nodes from the ground (+Z direction).
	\param	AT				Type of aggregation method to use.
	\param	out_scores		Output parameter for node scores.
	\param	out_scores_ptr	Pointer to the data of out_scores.
	\param	out_scores_size	Size of output_scores_ptr.

	\returns HF::Status::OK on completion.

	\par Caller's Responsibility
	The caller must call \link DestroyFloatVector \endlink with out_scores to free the memory allocated
	by this function.

	\pre 1) ERT points to a valid raytracer created by CreateRaytracer
	\pre 2) node_ptr contains a valid array of nodes with length equal to node_size
	\pre 3) out_scores, out_scores_ptr, and out_score_size are not null

	\post 1) out_scores contains a pointer to score for every node in nodes in order
	\post 2) out_scores_ptr cpoints to a valid array of scores
	\post 3) out_scores_size is updated to the length of the data held by out_scores_ptr

	\todo Is there any situation where out_scores_size is smaller than node_size?

	\see \ref raytracer_setup (how to create a BVH), \ref raytracer_teardown (how to destroy a BVH)

	\see \link SphericalViewAnalysis \endlink for an algorithm that returns the results of every ray casted instead
	of aggregating the results.

	\see \link SphericalViewAnalysisAggregateFlat \endlink for a function that works on a flat array of floats instead
	of an array of nodes.

	You must <b>load an .obj file</b> and <b>create a BVH</b> first.<br>
	Begin by reviewing the example at raytracer_setup before proceeding below.

	\par Example Code

	First, set up the parameters for the view analysis.
	\snippet tests\src\view_analysis_cinterface.cpp snippet_view_analysis_SphereicalViewAnalysisAggregate_setup_0

	Now you must prepare a pointer to a std::vector<float>, where the <b>aggregation results</b> will be stored.<br>
	You must also select the aggregate type.
	\snippet tests\src\view_analysis_cinterface.cpp snippet_view_analysis_SphereicalViewAnalysisAggregate_setup_1

	Now we are ready to call \link SphereicalViewAnalysisAggregate \endlink .
	\snippet tests\src\view_analysis_cinterface.cpp snippet_view_analysis_SphereicalViewAnalysisAggregate

	We can output the contents of the <b>aggregate results vector</b> to <b>stdout</b> .
	\snippet tests\src\view_analysis_cinterface.cpp snippet_view_analysis_SphereicalViewAnalysisAggregate_results

	After using the view analysis results, its resources must be <b>relinquished</b> .
	\snippet tests\src\view_analysis_cinterface.cpp snippet_view_analysis_SphereicalViewAnalysisAggregate_destroy

	From here, please review the example at \ref raytracer_teardown for instructions<br>
	on how to free the remainder of the resources used for the view analysis --<br>
	which are the (vector<\link HF::Geometry::MeshInfo \endlink> *) and (\link HF::Raytracer::EmbreeRayTracer \endlink *) instances.
*/
C_INTERFACE SphereicalViewAnalysisAggregate(
	HF::RayTracer::EmbreeRayTracer* ERT,
	HF::SpatialStructures::Node* node_ptr,
	int node_size,
	int max_rays,
	float upward_fov,
	float downward_fov,
	float height,
	AGGREGATE_TYPE AT,
	std::vector<float>** out_scores,
	float** out_scores_ptr,
	int* out_scores_size
);

/*!
	\brief	Conduct view analysis, and aggregate the results.

	\param	ERT				Raytracer containing the geometry to use for ray intersections.
	\param	node_ptr		Observer points for the view analysis. Each 3 floats represent the {x, y, z} of a new node.
	\param	node_size		Number of nodes in the array pointed to by node_ptr.

	\param	max_rays		Number of rays to cast for each node in node_ptr. 
							Note that this may fire fewer rays than max_rays depending on fov restrictions.

	\param	upward_fov		Maximum degrees upward from the viewer's eye level to consider.
	\param	downward_fov	Maximum degrees downward from the viewer's eye level to consider.
	\param	height			Height to offset nodes from the ground (+Z direction).
	\param	AT				Type of aggregation method to use.
	\param	out_scores		Output parameter for node scores.
	\param	out_scores_ptr	Pointer to the data of out_scores.
	\param	out_scores_size	Size of out_scores_ptr.
	
	\return HF::OK on completion.

	\details
	Similar to SphericalViewAnalysis but uses a flat array of floats instead
	of an array of nodes. This is to make the SphericalViewAnalysis function more
	accessible. 

	\par Caller's Responsibility
	The caller must call \link DestroyFloatVector \endlink with out_scores to free the 
	memory allocated by this function.

	\pre 1) ERT points to a valid raytracer created by CreateRaytracer.
	\pre 2) node_ptr contains a valid array of floats with length equal to node_size * 3.
	\pre 3) out_scores, out_scores_ptr, and out_score_size are not null.

	\post 1) out_scores contains a pointer to score for every node in nodes in order.
	\post 2) out_scores_ptr cpoints to a valid array of scores.
	\post 3) out_scores_size is updated to the length of the data held by out_scores_ptr.

	\todo Is there any situation where out_scores_size is smaller than node_size?

	\see	\ref raytracer_setup (how to create a BVH), \ref raytracer_teardown (how to destroy a BVH)

	You must <b>load an .obj file</b> and <b>create a BVH</b> first.<br>
	Begin by reviewing the example at \ref raytracer_setup before proceeding below.

	\par Example

	First, set up the parameters for the view analysis.
	\snippet tests\src\view_analysis_cinterface.cpp snippet_view_analysis_SphericalViewAnalysisAggregateFlat_setup_0

	Now you must prepare a pointer to a std::vector<float>, where the <b>aggregation results</b> will be stored.<br>
	You must also select the aggregate type.
	\snippet tests\src\view_analysis_cinterface.cpp snippet_view_analysis_SphericalViewAnalysisAggregateFlat_setup_1

	Now we are ready to call \link SphericalViewAnalysisAggregateFlat \endlink .
	\snippet tests\src\view_analysis_cinterface.cpp snippet_view_analysis_SphericalViewAnalysisAggregateFlat

	We can output the contents of the <b>aggregate results vector</b> to <b>stdout</b>.
	\snippet tests\src\view_analysis_cinterface.cpp snippet_view_analysis_SphericalViewAnalysisAggregateFlat_results

	After using the view analysis results, its resources must be <b>relinquished</b>.
	\snippet tests\src\view_analysis_cinterface.cpp snippet_view_analysis_SphericalViewAnalysisAggregateFlat_destroy

	From here, please review the example at \ref raytracer_teardown for instructions<br>
	on how to free the remainder of the resources used for the view analysis --<br>
	which are the (vector<\link HF::Geometry::MeshInfo \endlink> *) and (\link HF::Raytracer::EmbreeRayTracer \endlink *) instances.
*/
C_INTERFACE SphereicalViewAnalysisAggregateFlat(
	HF::RayTracer::EmbreeRayTracer* ERT,
	const float* node_ptr,
	int node_size,
	int max_rays,
	float upward_fov,
	float downward_fov,
	float height,
	AGGREGATE_TYPE AT,
	std::vector<float>** out_scores,
	float** out_scores_ptr,
	int* out_scores_size
);

/*!
	\brief	Perform view analysis, then get the distance and meshid for each individual ray casted.

	\param	ERT				Raytracer containing the geometry to use for ray intersections.
	\param	node_ptr		Observer points for the view analysis.
	\param	node_size		Number of nodes in the array pointed to by node_ptr.

	\param	max_rays		Number of rays to cast for each node in node_ptr. 
							This will be updated with the actual number of rays casted upon completion. 

	\param	upward_fov		Maximum degrees upward from the viewer's eye level to consider.
	\param	downward_fov	Maximum degrees downward from the viewer's eye level to consider.
	\param	height			Height to offset nodes from the ground (+Z direction).
	\param	out_results		Pointer to the vector containing the results of every ray casted.
	\param	out_results_ptr	Pointer to the data of out_results.

	\returns HF::OK on completion.

	\details
	Rays that do not intersect with any geometry will have a meshid of -1.

	\par Caller's Responsibility
	The caller must call \link DestroyRayResultVector \endlink with out_results to free the
	memory allocated by this function.

	\pre 1) ERT points to a valid raytracer created by \link CreateRaytracer \endlink.
	\pre 2) node_ptr contains a valid array of nodes with length equal to node_size.
	\pre 3) node_ptr, max_rays, and ERT are not null.

	\post 1) out_results contains a pointer to score for every node in nodes in order.
	\post 2) out_results_ptr points to a valid array of scores.
	\post 3) max_rays is updated to the number of rays casted in the view analysis.

	\see	\ref raytracer_setup (how to create a BVH), \ref raytracer_teardown (how to destroy a BVH)

	\see SphericalDistribute to get the direction of every ray casted by this function. Can be useful
	to determine the point of intersection for every ray casted.

	\par Example

	You must <b>load an .obj file</b> and <b>create a BVH</b> first.<br>
	Begin by reviewing the example at \ref raytracer_setup before proceeding below.

	First, set up the parameters for the view analysis.
	\snippet tests\src\view_analysis_cinterface.cpp snippet_view_analysis_SphericalViewAnalysisNoAggregate_setup_0

	Now you must prepare a pointer to a std::vector<\link RayResult \endlink>.<br>
	View analysis results will be stored at the memory addressed by this pointer.
	\snippet tests\src\view_analysis_cinterface.cpp snippet_view_analysis_SphericalViewAnalysisNoAggregate_setup_1

	Now we are ready to call \link SphericalViewAnalysisNoAggregate \endlink .
	\snippet tests\src\view_analysis_cinterface.cpp snippet_view_analysis_SphericalViewAnalysisNoAggregate

	We can output the contents of the <b>aggregate results vector</b> to <b>stdout</b>.
	\snippet tests\src\view_analysis_cinterface.cpp snippet_view_analysis_SphericalViewAnalysisNoAggregate_results

	After using the view analysis results, its resources must be <b>relinquished</b>.
	\snippet tests\src\view_analysis_cinterface.cpp snippet_view_analysis_SphericalViewAnalysisNoAggregate_destroy

	From here, please review the example at \ref raytracer_teardown for instructions<br>
	on how to free the remainder of the resources used for the view analysis --<br>
	which are the (vector<\link HF::Geometry::MeshInfo \endlink> *) and (\link HF::Raytracer::EmbreeRayTracer \endlink *) instances.

*/
C_INTERFACE SphericalViewAnalysisNoAggregate(
	HF::RayTracer::EmbreeRayTracer* ERT,
	const HF::SpatialStructures::Node* node_ptr,
	int node_size,
	int* max_rays,
	float upward_fov,
	float downward_fov,
	float height,
	std::vector<RayResult>** out_results,
	RayResult** out_results_ptr
);

/*!
	\brief	Perform view analysis, and get the distance and meshid for each individual ray casted.

	\param	ERT				Raytracer containing the geometry to use for ray intersections.

	\param	node_ptr		observer points for the view analysis. 
							Every 3 elements represents the x,y,z coordinates of a new point.

	\param	node_size		number of nodes in the array pointed to by node_ptr. 
							Should be equal to the length of the array contained by node_ptr * 3.

	\param	max_rays		number of rays to cast for each node in node_ptr. 
							This will be updated with the actual number of rays casted upon completion.

	\param	upward_fov		Maximum degrees upward from the viewer's eye level to consider.
	\param	downward_fov	Maximum degrees downward from the viewer's eye level to consider.
	\param	height			Height to offset nodes from the ground (+Z direction).
	\param	out_results		Pointer to the vector containing the results of every ray casted.
	\param	out_results_ptr	Pointer to the data of out_results.

	\returns HF::OK on completion.

	\details
	Rays that do not intersect with any geometry will have a meshid of -1.

	\par Caller's Responsibility
	The caller must call \link DestroyRayResultVector \endlink with out_results to free the
	memory allocated by this function.

	\pre 1) ERT points to a valid raytracer created by \link CreateRaytracer \endlink.
	\pre 2) node_ptr contains a valid array of floats with length equal to node_size *3 .
	\pre 3) node_ptr, max_rays, and ERT are not null.

	\post 1) out_scores contains a pointer to score for every node in nodes in order.
	\post 2) out_scores_ptr cpoints to a valid array of scores.
	\post 3) max_rays is updated to the number of rays casted in the view analysis.

	\see	\ref raytracer_setup (how to create a BVH), \ref raytracer_teardown (how to destroy a BVH)

	\see SphericalDistribute to get the direction of every ray casted by this function. Can be useful
	to determine the point of intersection for every ray casted.

	\par Example

	You must <b>load an .obj file</b> and <b>create a BVH</b> first.<br>
	Begin by reviewing the example at \ref raytracer_setup before proceeding below.

	First, set up the parameters for the view analysis.
	\snippet tests\src\view_analysis_cinterface.cpp snippet_view_analysis_SphericalViewAnalysisNoAggregateFlat_setup_0

	Now you must prepare a pointer to a std::vector<\link RayResult \endlink>.<br>
	View analysis results will be stored at the memory addressed by this pointer. 
	\snippet tests\src\view_analysis_cinterface.cpp snippet_view_analysis_SphericalViewAnalysisNoAggregateFlat_setup_1

	We are now ready to call \link SphericalViewAnalysisNoAggregateFlat \endlink.
	\snippet tests\src\view_analysis_cinterface.cpp snippet_view_analysis_SphericalViewAnalysisNoAggregateFlat

	We can output the contents of the <b>results vector</b> to <b>stdout</b>.
	\snippet tests\src\view_analysis_cinterface.cpp snippet_view_analysis_SphericalViewAnalysisNoAggregateFlat_results

	After using the view analysis results, its resources must be <b>relinquished</b>.
	\snippet tests\src\view_analysis_cinterface.cpp snippet_view_analysis_SphericalViewAnalysisNoAggregateFlat_destroy

	From here, please review the example at \ref raytracer_teardown for instructions<br>
	on how to free the remainder of the resources used for the view analysis --<br>
	which are the (vector<\link HF::Geometry::MeshInfo \endlink> *) and (\link HF::Raytracer::EmbreeRayTracer \endlink *) instances.
*/
C_INTERFACE SphericalViewAnalysisNoAggregateFlat(
	HF::RayTracer::EmbreeRayTracer* ERT,
	const float* node_ptr,
	int node_size,
	int* max_rays,
	float upward_fov,
	float downward_fov,
	float height,
	std::vector<RayResult>** out_results,
	RayResult** out_results_ptr
);

/*!
	\brief	Equally distribute points around a unit sphere.

	\param	num_rays				Number of points to distribute.

	\param	out_direction_vector	Output parameter for points generated. 
									Every 3 floats represents a new point.

	\param	out_direction_data		pointer to the data of out_direction_vector
	\param	upward_fov				Maximum degrees upward from the viewer's eye level to consider.
	\param	downward_fov			Maximum degrees downward from the viewer's eye level to consider.

	\returns HF::OK on completion.
	
	\par Caller's Responsibility
	The caller must call \link DestroyFloatVector \endlink with out_direction_vector to
	free the memory allocated by this function.

	\remarks
	This function is used internally by all view analysis functions to equally distribute
	the directions each ray will be casted in. 

	\pre out_direction_vector and out_direction_data must not be null, but the pointers they hold can be null.
	
	\post 1) The pointer pointed to by out_direction_vector contains a vector of directions.

	\post 2) num_rays contains the number of points distributed by this function. This may be more or less than
	the original number depending on the limitations specified by upward and downward fov.

	\par Example
	\code
		// Set arguments
		int num_rays = 10;
		std::vector<float>* out_float;
		float* out_float_data;
		float up_fov = 90.0f;
		float down_fov = 90.0f;

		// Call function
		auto status = SphericalDistribute(
			&num_rays,
			&out_float,
			&out_float_data,
			up_fov,
			down_fov
		);

		// Print results
		std::cerr << "Number of rays: " << num_rays << std::endl;
		for (int i = 0; i < num_rays; i++) {
			int os = i * 3;

			std::cerr << "("
				<< out_float_data[os] << ", "
				<< out_float_data[os+1] << ", "
				<< out_float_data[os+2] << ")"
				<< std::endl;
		}

		// Deallocate Memory
		DestroyFloatVector(out_float);
	\endcode

	`>>> Number of rays: 10`\n
	`>>> (-0, -1, 0)`\n
	`>>> (-0.276545, -0.8, -0.532469)`\n
	`>>> (0.751457, -0.6, 0.27443)`\n
	`>>> (-0.847177, -0.4, 0.349703)`\n
	`>>> (0.415282, -0.2, -0.887435)`\n
	`>>> (0.299284, 0, 0.954164)`\n
	`>>> (-0.847731, 0.2, -0.491275)`\n
	`>>> (0.895138, 0.4, -0.196795)`\n
	`>>> (-0.460102, 0.6, 0.654451)`\n
	`>>> (-0.0771074, 0.8, -0.595025)`\n

*/
C_INTERFACE SphericalDistribute(
	int* num_rays,
	std::vector<float>** out_direction_vector,
	float** out_direction_data,
	float upward_fov,
	float downward_fov
);

/**@}*/
