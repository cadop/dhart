///
///	\file		visibility_graph.h
///	\brief		Contains definitions for the <see cref = "HF::AnalysisMethods::VisibilityGraph">VisibilityGraph</cref> class
///
///	\author		TBA
///	\date		17 Jun 2020

#include <vector>

// Forward Declares
namespace HF {
	namespace SpatialStructures {
		class Graph;			///< see graph.h in spatialstructures
		struct Node;			///< see node.h in spatialstructures
		struct Edge;			///< see Edge.h in spatialstructures
	}

	namespace RayTracer {
		class EmbreeRayTracer;	///< see embree_raytracer.h in raytracer
	}
}

namespace HF {
	namespace AnalysisMethods {
		namespace VisibilityGraph {
			/// <summary>
			/// All variations for generating the visibility graph 
			/// </summary>

			/// <summary>
			/// Generate a visibility graph between all nodes in input nodes
			/// </summary>
			/// <param name="ert">reference to EmbreeRayTracer for the VisibilityGraph</param>
			/// <param name="input_nodes">reference to vector of Node for VisibilityGraph</param>
			/// <param name="height">Desired height for the VisibilityGraph</param>

			/*!
				\code
					// be sure to #include "objloader.h"

					// path to OBJ file for a flat plane
					std::string plane_path = "plane.obj";

					// Create a vector of MeshInfo from plane_path, using LoadMeshObjects.
					// Note that LoadMeshObjects has two more arguments after plane_path -
					//		an enum HF::Geometry::GROUP_METHOD (defaults to GROUP_METHOD::ONLY_FILE)
					//		a bool (defaults to true, used to convert OBJ coordinate system to Rhino coordinates)
					std::vector<HF::Geometry::MeshInfo> meshInfo = HF::Geometry::LoadMeshObjects(plane_path);

					// Create an EmbreeRayTracer.
					HF::RayTracer::EmbreeRayTracer tracer(meshInfo);

					std::vector<HF::SpatialStructures::Node> node_vec;		// container of nodes
					const int reserve_count = 100;							// pre-defined reserve size
					node_vec.reserve(reserve_count);						// reserve reserve_count blocks for node_vec

					// Construct reserve_count Node and insert each of them into node_vec
					for (float i = -5.0; i < 5.0; i++) {
						// runs 10 times
						for (float j = -5; j < 5.0; j++) {
							// runs 10 times
							node_vec.emplace_back(Node(i, j, 0.0f));	// all Node ID default to -1
						}
					}

					// AllToAll constructs and returns a Graph
					// consisting of Node (from node_vec) that do not occlude each other
					float desired_height = 2.0f;		// Height of graph
					HF::SpatialStructures::Graph graph = AllToAll(tracer, node_vec, desired_height);
				\endcode
			*/
			HF::SpatialStructures::Graph AllToAll(
				HF::RayTracer::EmbreeRayTracer& ert,
				const std::vector<HF::SpatialStructures::Node>& input_nodes,
				float height = 1.7f
			);

			/// <summary>
			/// Generate a visibility graph between every node in set a and every node in set b
			/// </summary> 
			/// <param name="ert">reference to EmbreeRayTracer for the VisibilityGraph</param>
			/// <param name="input_nodes">reference to vector of Node for VisibilityGraph</param>
			/// <param name="height">Desired height for the VisibilityGraph</param>

			/*!
				\code
					// be sure to #include "objloader.h"

					// path to OBJ file for a flat plane
					std::string plane_path = "plane.obj";

					// Create a vector of MeshInfo from plane_path, using LoadMeshObjects.
					// Note that LoadMeshObjects has two more arguments after plane_path -
					//		an enum HF::Geometry::GROUP_METHOD (defaults to GROUP_METHOD::ONLY_FILE)
					//		a bool (defaults to true, used to convert OBJ coordinate system to Rhino coordinates)
					std::vector<HF::Geometry::MeshInfo> meshInfo = HF::Geometry::LoadMeshObjects(plane_path);

					// Create an EmbreeRayTracer.
					HF::RayTracer::EmbreeRayTracer tracer(meshInfo);

					std::vector<HF::SpatialStructures::Node> node_vec_0;		// First container of Node
					std::vector<HF::SpatialStructures::Node> node_vec_1;		// Second container of Node

					const int reserve_count = 100;		// Pre-defined reserve size
					node_vec_0.reserve(reserve_count);	// Reserve reserve_count blocks for both vectors
					node_vec_1.reserve(reserve_count);

					// 0) Construct reserve_count Node and insert each of them into node_vec_0
					for (float i = -5.0; i < 5.0; i++) {
						// runs 10 times
						for (float j = -5.0; j < 5.0; j++) {
							// runs 10 times
							node_vec_0.emplace_back(HF::SpatialStructures::Node(i, j, 0.0f));	// all Node ID default to -1
						}
					}

					// 1) Construct reserve_count Node and insert each of them into node_vec_1
					for (float i = 0.0; i < 10.0; i++) {
						for (float j = 0.0; j < 10.0; j++) {
							node_vec_1.emplace_back(HF::SpatialStructures::Node(i, j, 0.0f));	// all Node ID default to -1
						}
					}

					// GroupToGroup constructs and returns a Graph consisting of Node
					// (between node_vec_0 and node_vec_1) such that the nodes do not occlude each other
					float desired_height = 2.0f;		// Height of graph
					HF::SpatialStructures::Graph graph = HF::AnalysisMethods::VisibilityGraph::GroupToGroup(tracer, node_vec_0, node_vec_1, desired_height);
				\endcode
			*/
			HF::SpatialStructures::Graph GroupToGroup(
				HF::RayTracer::EmbreeRayTracer& ert,
				const std::vector<HF::SpatialStructures::Node> & from,
				const std::vector<HF::SpatialStructures::Node> & to,
				float height = 1.7f
			);

			/// <summary>
			/// Parallel AllToAll Algorithm for an undirected visibility graph
			/// </summary>
			/// <param name="ert">reference to EmbreeRayTracer for the VisibilityGraph</param>
			/// <param name="input_nodes">reference to vector of Node for VisibilityGraph</param>
			/// <param name="height">Desired height for the VisibilityGraph</param>
			/// <param name="cores">Core count for the graph</param>

			/*!
				\code
					// be sure to #include "objloader.h"

					// path to OBJ file for a flat plane
					std::string plane_path = "plane.obj";

					// Create a vector of MeshInfo from plane_path, using LoadMeshObjects.
					// Note that LoadMeshObjects has two more arguments after plane_path -
					//		an enum HF::Geometry::GROUP_METHOD (defaults to GROUP_METHOD::ONLY_FILE)
					//		a bool (defaults to true, used to convert OBJ coordinate system to Rhino coordinates)
					std::vector<HF::Geometry::MeshInfo> meshInfo = HF::Geometry::LoadMeshObjects(plane_path);

					// Create an EmbreeRayTracer.
					HF::RayTracer::EmbreeRayTracer tracer(meshInfo);

					std::vector<HF::SpatialStructures::Node> node_vec;		// container of nodes
					const int reserve_count = 100;							// pre-defined reserve size
					node_vec.reserve(reserve_count);						// reserve reserve_count blocks for node_vec

					// Construct reserve_count Node and insert each of them into node_vec
					for (float i = -5.0; i < 5.0; i++) {
						// runs 10 times
						for (float j = -5; j < 5.0; j++) {
							// runs 10 times
							node_vec.emplace_back(HF::SpatialStructures::Node(i, j, 0.0f));	// all Node ID default to -1
						}
					}

					// AllToAllUndirected constructs and returns a Graph
					// consisting of Node (from node_vec) that do not occlude each other
					float desired_height = 2.0f;		// Height of graph
					int core_count = 4;					// For omp_set_num_threads(int num_threads), CPU core count
					HF::SpatialStructures::Graph graph =
						HF::AnalysisMethods::VisibilityGraph::AllToAllUndirected(tracer, node_vec, desired_height, core_count);
				\endcode
			*/
			HF::SpatialStructures::Graph AllToAllUndirected(
				HF::RayTracer::EmbreeRayTracer& ert,
				const std::vector<HF::SpatialStructures::Node>& nodes,
				float height,
				int cores = -1
			);
		}
	}
}