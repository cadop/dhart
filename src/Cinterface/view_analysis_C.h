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
	\param	node_ptr		observer points for the view analysis.
	\param	node_size		number of nodes in the array pointed to by node_ptr

	\param	max_rays		number of rays to cast for each node in node_ptr. 
							Note that this may fire fewer rays than max_rays depending on fov restrictions.

	\param	upward_fov		Maximum degrees upward from the viewer's eye level to consider.
	\param	downward_fov	Maximum degrees downward from the viewer's eye level to consider. 
	\param	height			Height to offset nodes from the ground (+Z direction)
	\param	AT				Type of aggregation method to use
	\param	out_scores		Output parameter for node scores
	\param	out_scores_ptr	pointer to the data of out_scores
	\param	out_scores_size	Size of output scores_ptr

	\returns HF::Status::OK on completion.

	\par Caller's Responsibility
	The caller must call \link DestroyFloatVector \endlink with out_scores to free the memory allocated
	by this function.

	\pre 1) ERT points to a valid raytracer created by CreateRaytracer
	\pre 2) node_ptr contains a valid array of nodes with length equal to node_size
	\pre 3) out_scores, out_scores_ptr, and out_score_size are not null

	\post 1) out_scores contains a pointer  to score for every node in nodes in order
	\post 2) out_scores_ptr cpoints to a valid array of scores
	\post 3) out_scores_size is updated to the length of the data held by out_scores_ptr

	\todo Is there any situation where out_scores_size is smaller than node_size?
	
	\see \link SphericalViewAnalysis \endlink for an algorithm that returns the results of every ray casted instead
	of aggregating the results.

	\see \link SphericalViewAnalysisAggregateFlat \endlink for a function that works on a flat array of floats instead
	of an array of nodes.

	\par Example Code
	\code 
		// Create Plane
		const std::vector<float> plane_vertices{
			-10.0f, 10.0f, 0.0f,
			-10.0f, -10.0f, 0.0f,
			10.0f, 10.0f, 0.0f,
			10.0f, -10.0f, 0.0f,
		};
		const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

		// Create and allocate a new instacnce of meshinfo
		std::vector<MeshInfo> * MI;
		auto MIR = StoreMesh(
			&MI,
			plane_indices.data(),
			plane_indices.size(),
			plane_vertices.data(),
			plane_vertices.size(),
			"",
			0
		);

		// Create a new raytracer
		EmbreeRayTracer * ert;
		CreateRaytracer(MI, &ert);

		// Create Nodes
		std::vector<Node> nodes = {
			Node(0,0,1),
			Node(0,0,2),
			Node(0,0,3),
		};

		// Set values for arguments
		float max_rays = 10000;
		float up_fov = 90;
		float down_fov = 90;
		float height = 1.7f;
		AGGREGATE_TYPE AT = AGGREGATE_TYPE::AVERAGE;

		std::vector<float>* scores;
		float* scores_ptr;
		int scores_size;

		// Run View Analysis
		auto result = SphereicalViewAnalysisAggregate(
			ert,
			nodes.data(),
			nodes.size(),
			max_rays,
			up_fov,
			down_fov,
			height,
			AT,
			&scores,
			&scores_ptr,
			&scores_size
		);

		// print Results
		for (int i = 0; i < scores->size(); i++)
			std::cerr << (*scores)[i] << std::endl;

		// Deallocate Memory
		DestroyFloatVector(scores);
		DestroyMeshInfo(MI);
		DestroyRayTracer(ert);
	\endcode

	`>>> 4.28035`\n
	`>>> 5.2776`\n
	`>>> 6.23221`\n
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
	\param	node_ptr		Observer points for the view analysis. Each 3 floats represent the x,y,z of a new node.
	\param	node_size		number of nodes in the array pointed to by node_ptr

	\param	max_rays		number of rays to cast for each node in node_ptr. 
							Note that this may fire fewer rays than max_rays depending on fov restrictions.

	\param	upward_fov		Maximum degrees upward from the viewer's eye level to consider.
	\param	downward_fov	Maximum degrees downward from the viewer's eye level to consider.
	\param	height			Height to offset nodes from the ground (+Z direction)
	\param	AT				Type of aggregation method to use
	\param	out_scores		Output parameter for node scores
	\param	out_scores_ptr	pointer to the data of out_scores
	\param	out_scores_size	number of elements of output scores_ptr
	
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

	\par Example
	\code
		// Create Plane
		const std::vector<float> plane_vertices{
			-10.0f, 10.0f, 0.0f,
			-10.0f, -10.0f, 0.0f,
			10.0f, 10.0f, 0.0f,
			10.0f, -10.0f, 0.0f,
		};
		const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

		// Create and allocate a new instacnce of meshinfo
		std::vector<MeshInfo>* MI;
		auto MIR = StoreMesh(
			&MI,
			plane_indices.data(),
			plane_indices.size(),
			plane_vertices.data(),
			plane_vertices.size(),
			"",
			0
		);

		// Create a new raytracer
		EmbreeRayTracer* ert;
		CreateRaytracer(MI, &ert);

		// Create Nodes
		std::vector<float> nodes = {
			0,0,1,
			0,0,2,
			0,0,3,
		};

		// Set values for arguments
		float max_rays = 10000;
		float up_fov = 90;
		float down_fov = 90;
		float height = 1.7f;
		AGGREGATE_TYPE AT = AGGREGATE_TYPE::AVERAGE;

		std::vector<float>* scores;
		float* scores_ptr;
		int scores_size;

		// Run View Analysis
		auto result = SphereicalViewAnalysisAggregateFlat(
			ert,
			nodes.data(),
			nodes.size()/3,
			max_rays,
			up_fov,
			down_fov,
			height,
			AT,
			&scores,
			&scores_ptr,
			&scores_size
		);

		// print Results
		for (int i = 0; i < scores->size(); i++)
			std::cerr << (*scores)[i] << std::endl;

		// Deallocate Memory
		DestroyFloatVector(scores);
		DestroyMeshInfo(MI);
		DestroyRayTracer(ert);

	\endcode

	`>>> 4.28035`\n
	`>>> 5.2776`\n
	`>>> 6.23221`\n

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
	\param	node_ptr		observer points for the view analysis.
	\param	node_size		number of nodes in the array pointed to by node_ptr

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
	\pre 2) node_ptr contains a valid array of nodes with length equal to node_size.
	\pre 3) node_ptr, max_rays, and ERT are not null.

	\post 1) out_results contains a pointer to score for every node in nodes in order.
	\post 2) out_results_ptr points to a valid array of scores.
	\post 3) max_rays is updated to the number of rays casted in the view analysis.

	\see SphericalDistribute to get the direction of every ray casted by this function. Can be useful
	to determine the point of intersection for every ray casted.

	\par Example
	\code
		// Create Plane
		const std::vector<float> plane_vertices{
			-10.0f, 10.0f, 0.0f,
			-10.0f, -10.0f, 0.0f,
			10.0f, 10.0f, 0.0f,
			10.0f, -10.0f, 0.0f,
		};
		const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

		// Create and allocate a new instacnce of meshinfo
		std::vector<MeshInfo>* MI;
		auto MIR = StoreMesh(
			&MI,
			plane_indices.data(),
			plane_indices.size(),
			plane_vertices.data(),
			plane_vertices.data(),
			plane_vertices.size(),
			"",
			0
		);

		// Create a new raytracer
		EmbreeRayTracer* ert;
		CreateRaytracer(MI, &ert);

		// Create Nodes
		std::vector<Node> nodes = {
			Node(0,0,1),
			Node(0,0,2),
			Node(0,0,3),
		};

		// Set values for arguments
		int max_rays = 10;
		float up_fov = 90;
		float down_fov = 90;
		float height = 1.7f;
		AGGREGATE_TYPE AT = AGGREGATE_TYPE::AVERAGE;

		std::vector<RayResult>* results;
		RayResult* results_ptr;

		// Run View Analysis
		auto result = SphericalViewAnalysisNoAggregate(
			ert,
			nodes.data(),
			nodes.size(),
			&max_rays,
			up_fov,
			down_fov,
			height,
			&results,
			&results_ptr
		);

		// print Results
		std::cerr << "Num Rays: " << max_rays << std::endl;
		for (int i = 0; i < nodes.size(); i++) {
			std::cerr << "Node " << i << ": ";
			for (int k = 0; k < results->size()/3; k++) {
				int os = max_rays * i;
				std::cerr << "(" << results_ptr[k + os].meshid
					<< ", " << results_ptr[k + os].distance << "), ";
			}
			std::cerr << std::endl;
		}

		std::cerr << std::endl;

		// Deallocate Memory
		DestroyRayResultVector(results);
		DestroyMeshInfo(MI);
		DestroyRayTracer(ert);
		
	\endcode

	`>>> Num Rays: 10`\n
	`>>> Node 0: (-1, -1), (0, 5.07072), (-1, -1), (-1, -1), (0, 3.04248), (-1, -1), (0, 5.4959), (-1, -1), (-1, -1), (0, 4.53763),`\n
	`>>> Node 1: (-1, -1), (0, 6.94876), (-1, -1), (-1, -1), (0, 4.16932), (-1, -1), (0, 7.53142), (-1, -1), (-1, -1), (0, 6.21823),`\n
	`>>> Node 2: (-1, -1), (0, 8.82681), (-1, -1), (-1, -1), (0, 5.29616), (-1, -1), (0, 9.56693), (-1, -1), (-1, -1), (0, 7.89883),`\n
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

	\see SphericalDistribute to get the direction of every ray casted by this function. Can be useful
	to determine the point of intersection for every ray casted.

	\par Example
	\code 
		// Create Plane
		const std::vector<float> plane_vertices{
			-10.0f, 10.0f, 0.0f,
			-10.0f, -10.0f, 0.0f,
			10.0f, 10.0f, 0.0f,
			10.0f, -10.0f, 0.0f,
		};
		const std::vector<int> plane_indices{ 3, 1, 0, 2, 3, 0 };

		// Create and allocate a new instacnce of meshinfo
		std::vector<MeshInfo>* MI;
		auto MIR = StoreMesh(
			&MI,
			plane_indices.data(),
			plane_indices.size(),
			plane_vertices.data(),
			plane_vertices.size(),
			"",
			0
		);

		// Create a new raytracer
		EmbreeRayTracer* ert;
		CreateRaytracer(MI, &ert);

		// Create Nodes
		std::vector<float> nodes = {
			0,0,1,
			0,0,2,
			0,0,3,
		};

		// Set values for arguments
		int max_rays = 10;
		float up_fov = 90;
		float down_fov = 90;
		float height = 1.7f;
		AGGREGATE_TYPE AT = AGGREGATE_TYPE::AVERAGE;

		std::vector<RayResult>* results;
		RayResult * results_ptr;

		// Run View Analysis
		auto result = SphericalViewAnalysisNoAggregateFlat(
			ert,
			nodes.data(),
			nodes.size() / 3,
			&max_rays,
			up_fov,
			down_fov,
			height,
			&results,
			&results_ptr
		);

		// print Results
		std::cerr << "Num Rays: " << max_rays << std::endl;
		for (int i = 0; i < nodes.size() / 3; i++) {
			std::cerr << "Node " << i << ": ";
			for (int k = 0; k < results->size() / 3; k++) {
				int os = max_rays * i;
				std::cerr << "(" << results_ptr[k+os].meshid
				<< ", " << results_ptr[k+os].distance << "), ";
			}
			std::cerr << std::endl;
		}

		std::cerr << std::endl;

		// Deallocate Memory
		DestroyRayResultVector(results);
		DestroyMeshInfo(MI);
		DestroyRayTracer(ert);
	\endcode

	`>>> Num Rays: 10`\n
	`>>> Node 0: (-1, -1), (0, 5.07072), (-1, -1), (-1, -1), (0, 3.04248), (-1, -1), (0, 5.4959), (-1, -1), (-1, -1), (0, 4.53763),`\n
	`>>> Node 1: (-1, -1), (0, 6.94876), (-1, -1), (-1, -1), (0, 4.16932), (-1, -1), (0, 7.53142), (-1, -1), (-1, -1), (0, 6.21823),`\n
	`>>> Node 2: (-1, -1), (0, 8.82681), (-1, -1), (-1, -1), (0, 5.29616), (-1, -1), (0, 9.56693), (-1, -1), (-1, -1), (0, 7.89883),`\n

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
