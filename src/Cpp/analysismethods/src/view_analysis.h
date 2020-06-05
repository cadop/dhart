#include <vector>
#include <array>
#include <cmath>
#include <iostream>
#include <assert.h>

#ifndef VIEW_ANALYSIS_G
#define VIEW_ANALYSIS_G

//Forward Declares
namespace HF {
	namespace SpatialStructures {
		struct Node;
	}
	namespace RayTracer {
		class EmbreeRayTracer;
		struct FullRayRequest;
	}
}

namespace HF::AnalysisMethods::ViewAnalysis {
	/// <summary>
	/// The type of aggregation to be used for a specific algorithm
	/// </summary>
	enum class AGGREGATE_TYPE {
		COUNT = 0,
		SUM = 1,
		AVERAGE = 2,
		MAX = 3,
		MIN = 4
	};

	/// <summary>
	/// Evenly distribute a set of points around a sphere centered at the origin.
	/// </summary>
	/// <param name="num_points">Maximum number of points to distribute</param>
	/// <param name="upward_limit"> Cull points beyond this upward angle </param>
	/// <param name="downward_limit"> Cull poitns beyond this downward angle </param>
	/// <returns>x, y, and z coordinates of each point on the sphere</returns>
	std::vector<std::array<float, 3>> FibbonacciDistributePoints(
		int num_points,
		float upwards_fov = 50.0f,
		float downward_fov = 70.0f
	);

	/// <summary>
	/// Conduct view analysis and get ray requests as the result
	/// </summary>
	/// <param name="ert">The raytracer to shoot rays with</param>
	/// <param name="nodes">The nodes to shoot rays from</param>
	/// <param name="max_rays">Maximum number of rays to shoot per node</param>
	/// <param name="max_rays">Nodes will be offset by this height before being analyzed</param>
	/// <returns>A vector of rayrequests containing the output</returns>
	std::vector<std::vector<HF::RayTracer::FullRayRequest>> SphericalViewAnalysisWithRayRequests(
		HF::RayTracer::EmbreeRayTracer& ert,
		const std::vector<HF::SpatialStructures::Node>& nodes,
		int max_rays,
		float height = 1.7f
	);

	/// <summary>
	/// Apply the desired aggregate function to the value;
	/// </summary>
	/// <param name="out_total">reference to float</param>
	/// <param name="new_value">value of float</param>
	/// <param name="agg_type">AGGREGATE_TYPE - valid range is [0, 4]</param>
	/// <param name="count">count value to use</param>
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
		case AGGREGATE_TYPE::MAX:
			out_total = std::max<float>(out_total, new_value);
			break;
		case AGGREGATE_TYPE::MIN:
			out_total = std::min<float>(out_total, new_value);
			break;
		default:
			throw std::out_of_range("Unimplemented aggregation type");
			break;
		}
		assert(out_total == 0 || isnormal(out_total));
		return;
	}
	/// <summary>
	///	Conduct view analysis with hardcoded embree raytracer
	/// </summary>
	/// <param name="ert">The raytracer to shoot rays with</param>
	/// <param name="nodes">The nodes to shoot rays from</param>
	/// <param name="max_rays">Maximum number of rays to shoot per node</param>
	/// <param name="max_rays">Nodes will be offset by this height before being analyzed</param>
	/// <returns>A vector of rayrequests containing the output</returns>
	std::vector<float> SphericalViewAnalysisAverageDistance(
		HF::RayTracer::EmbreeRayTracer& ert,
		const std::vector<HF::SpatialStructures::Node> Nodes,
		int max_rays,
		float height = 1.7f
	);

	/// <summary>
	/// Conduct view analysis. RES must have a .SetHit() method.
	/// out_directions is filled with directions if a non-null pointer is supplied
	/// </summary>
	/// <param name="ray_tracer"> A valid raytracer that has an intersect function </param>
	/// <param name="Nodes"> A list of nodes with atleast (x,y,z) coordinates </param>
	/// <param name="num_rays"> The number of rays to shoot in the view analysis </param>
	/// <param name="height"> Height off the ground to fire from</param>
	/// <returns>A vector of typename RES</returns>
	template <typename RES, typename RT, typename N>
	std::vector<RES> SphericalViewAnalysis(
		RT& ray_tracer,
		const std::vector<N>& Nodes,
		int num_rays,
		float upward_limit = 50.0f,
		float downward_limit = 70.0f,
		float height = 1.7f
	) {
		// Perform a quick check to see if we can even hold this vector
		const auto directions = FibbonacciDistributePoints(num_rays, upward_limit, downward_limit);
		int required_vector_size = directions.size() * Nodes.size();
		std::vector<RES> out_results;
		if (out_results.max_size() < required_vector_size) {
			std::cerr << "The desired view analysis setting exceed the maximum vector size!" << std::endl;
			throw std::bad_array_new_length();
		}
		out_results.resize(required_vector_size);
		float out_distance = 0;	int out_mid = 0;
		num_rays = directions.size();
#pragma omp parallel for schedule(dynamic)
		for (int i = 0; i < Nodes.size(); i++) {
			auto node = Nodes[i];
			node[2] += height;
			int os = directions.size() * i;

			// Iterate through every direction and fire a ray for it
			for (int k = 0; k < directions.size(); k++)
				if (ray_tracer.FireAnyRay(node, directions[k], out_distance, out_mid))
					out_results[os + k].SetHit(node, directions[k], out_distance, out_mid);
		}

		return out_results;
	}

	/// <summary>
	/// Conduct View Analysis
	/// </summary>
	/// <param name="ray_tracer"> A raytracer loaded with geometry that has a .FireRay() function </param>
	/// <param name="Nodes"> Any class/struct with [] defined for 0, 1, and 2 </param>
	/// <param name="num_rays"> The number of rays to fire, evenly distributed in a sphere </param>
	/// <param name="height"> Nodes will be raised this distance off the ground before being evaluated </param>
	/// <param name="aggregation"> Aggregation method to use </param>
	/// <returns> A vector of float (TODO: better description) </returns>
	template<typename RT, typename N>
	std::vector<float> SphericalRayshootWithAnyRTForDistance(
		RT& ray_tracer,
		const std::vector<N>& Nodes,
		int num_rays,
		float upward_limit = 50.0f,
		float downward_limit = 70.0f,
		float height = 1.7f,
		const AGGREGATE_TYPE aggregation = AGGREGATE_TYPE::SUM
	) {
		std::vector<float> out_scores(Nodes.size());
		const auto directions = FibbonacciDistributePoints(num_rays, upward_limit, downward_limit);

#pragma omp parallel for schedule(dynamic)
		for (int i = 0; i < Nodes.size(); i++) {
			const auto& node = Nodes[i];
			
			float& score = out_scores[i];
			score = 0;
			int count = 1;
			for (const auto& direction : directions)
			{
				auto node_copy = node;
				node_copy[2] += height;
				float distance = 0;
				if (ray_tracer.FireRay(
					node_copy[0], node_copy[1], node_copy[2],
					direction[0], direction[1], direction[2]
				)) {
					distance = sqrtf(
						powf((node[0] - node_copy[0]), 2) +
						powf((node[1] - node_copy[1]), 2) +
						powf((node[2] - node_copy[2]), 2)
					);
					Aggregate(score, distance, aggregation, count);
					count++;
				}
			}
		}
		return out_scores;
	}
}

#endif
