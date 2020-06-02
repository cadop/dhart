#include <view_analysis.h>

#include <vector>
#include <array>
#include <cmath>

#include <embree_raytracer.h>
#include <RayRequest.h>
#include <node.h>


using namespace HF::RayTracer;
using std::vector;
using HF::SpatialStructures::Node;
#undef min
#undef max
namespace HF::AnalysisMethods::ViewAnalysis {

	constexpr float ConvertToRadians(float num_in_degrees) {
		return num_in_degrees * (static_cast<float>(M_PI) / 180.00f);
	}

	/// <summary>
	/// Normalize the given vector
	/// </summary>
	inline void Normalize(std::array<float, 3> & vec) {
		float& x = vec[0];
		float& y = vec[1];
		float& z = vec[2];

		float magnitude = sqrt(
			pow(x, 2) + pow(y, 2) + pow(z, 2)
		);
		vec[0] /= magnitude;
		vec[1] /= magnitude;
		vec[2] /= magnitude;

	}

	/*float maximum_phi = std::numeric_limits<float>::min();
	float minimum_phi = std::numeric_limits<float>::max();
	float new_maximum_phi = std::numeric_limits<float>::min();
	float new_minimum_phi = std::numeric_limits<float>::max();
	*/
	inline bool CapAltitude(std::array<float, 3>& vec, float max_angle, float min_angle) {
		
		float& x = vec[0]; float& y = vec[1]; float& z = vec[2];
		
		// Convert to spherical coordinates
		float r = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
		float theta = atan2(y, x);
		//float phi = atan(sqrt(pow(x, 2) + pow(y, 2)) / z);
		float phi = acos(z / r);


		//maximum_phi = std::max(phi, maximum_phi);
		//minimum_phi = std::min(phi, minimum_phi);

		// Lerp phi
		//printf("Old Phi: %f, ", phi);
		phi = min_angle + (phi/static_cast<float>(M_PI)) * (max_angle - min_angle);
		//printf("New Phi: %f\n", phi);
		
	//	new_maximum_phi = std::max(phi, new_maximum_phi);
	//	new_minimum_phi = std::min(phi, new_minimum_phi);

		x = r * sin(phi) * cos(theta);
		y = r * sin(phi) * sin(theta);
		z = r * (cos(phi));


		//printf("(%f,%f,%f)\n", x,y,z);

	}

	inline bool AltitudeWithinRange(const std::array<float, 3>& vec, float max_angle, float min_angle) {

		float x = vec[0]; float y = vec[1]; float z = vec[2];

		float r = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
		float phi = acos(z / r);

		return (phi >= min_angle && phi <= max_angle);
	}


	vector<std::array<float, 3>> FibbonacciDist(
		int num_points,
		float upwards_fov,
		float downward_fov
	) {

		float upperlimit = ConvertToRadians(upwards_fov);
		float lowerlimit = ConvertToRadians(downward_fov);

		float max_phi = static_cast<float>(M_PI_2) + lowerlimit;
		float min_phi = static_cast<float>(M_PI_2) - upperlimit;

		int n = num_points;
		std::vector<std::array<float, 3>> out_points;
		float offset = 2.0f / float(n);
		const float increment = static_cast<float>(M_PI)* (3.0f - sqrtf(5.0f));

		for (int i = 5; i < n + 5; i++) {
			float y = ((float(i) * offset) - 1.0f) - (offset / 0.2f);
			float r = sqrtf(1.0f - powf(y, 2));
			//r = min_phi + r * (max_phi - min_phi);
			float phi = (float(i + 1) * increment);
			float x = cosf(phi) * r;
			float z = sinf(phi) * r;

			//float angle = glm::angle(glm::vec3(x, y, z), axis);
			if (!isnan(x) && !isnan(y) && !isnan(z)) {// && angle > upperlimit && angle < lowerlimit)
				auto new_point = std::array<float, 3>{x, y, z};
				Normalize(new_point);
				if (AltitudeWithinRange(new_point, max_phi, min_phi))
					out_points.emplace_back(new_point);
				//CapAltitude(out_points.back(), max_phi, min_phi);
			}
		}
		//printf("Out Points Size: %d\n", out_points.size());
		return out_points;

	}

	vector<std::array<float, 3>> FibbonacciDistributePoints(
		int num_points,
		float upwards_fov,
		float downward_fov
	){

		auto out_points = FibbonacciDist(num_points, upwards_fov, downward_fov);
		//printf("PHI min: %f, max: %f\n", minimum_phi, maximum_phi);
		//printf("NEWPHI min: %f, max: %f\n", new_minimum_phi, new_maximum_phi);
		//printf("Limits min: %f, max: %f\n", min_phi, max_phi);

		int points_removed = num_points - out_points.size();
		double percent_removed = static_cast<double>(points_removed) / static_cast<double>(num_points);
		
		/*
		printf(
			"Radians removed: %f, Points removed %d, Percent per radian: %f\n", 
			(M_PI - max_phi) + min_phi, 
			num_points - out_points.size(), 
			percent_removed_per_degree
		);

		printf(
			"Max points: %d, Total points:%d, Percentage = %f\n",
			num_points,
			out_points.size(),
			percent_removed
		);
		*/
		int new_out_points = (num_points / (1.0 - percent_removed));
		out_points.clear();
		return FibbonacciDist(new_out_points, upwards_fov, downward_fov);
	}


	std::vector<float> SphericalViewAnalysisWithDistance(HF::RayTracer::EmbreeRayTracer& ert, const std::vector<HF::SpatialStructures::Node> Nodes, int max_rays, float height)
	{
		return std::vector<float>();
	}

	vector<vector<FullRayRequest>> SphericalViewAnalysisFromRayRequests(
		EmbreeRayTracer& ert, 
		const vector<Node>& nodes,
		int max_rays,
		float height
	){
		const auto directions = FibbonacciDistributePoints(max_rays);
		
		// Preallocate an array of requests for each node
		int num_directions = nodes.size();
		int num_nodes = directions.size();
		vector<vector<FullRayRequest>> out_requests(num_nodes, vector<FullRayRequest>(num_directions));
		
		for (int i = 0; i < num_nodes; i++) {
			auto node_a = nodes[i];
			auto origin_a = node_a.getArray();

			auto& requests_for_this_node = out_requests[i];
			FullRayRequest r(origin_a[0], origin_a[1], origin_a[2], -1,-1,-1,-1);
			
			for (int k = 0; k < directions.size(); k++) {
				const auto & dir = directions[k];
				auto& this_request = requests_for_this_node[k];
				
				this_request.dx = dir[0];
				this_request.dy = dir[1];
				this_request.dz = dir[2];
			}
			ert.FireRequests(requests_for_this_node);
		}
		return out_requests;
	}
}
