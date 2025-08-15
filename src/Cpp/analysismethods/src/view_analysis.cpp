///
/// \file		view_analysis.cpp
/// \brief		Contains implementation for the <see cref="HF::ViewAnalysis">ViewAnalysis</see> namespace
///
///	\author		TBA
///	\date		26 Jun 2020

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
namespace HF::ViewAnalysis {

	/// Convert a number from degrees to radians. 
	constexpr float ConvertToRadians(float num_in_degrees) {
		return num_in_degrees * (static_cast<float>(M_PI) / 180.00f);
	}

	/// Normalize a vector.
	/// \param vec Vector to normalize. Will be updated with the normalized value.
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

	/*!
		Check if a vector's altitude is between max and min angle.

		\param vec Vector to check the altitude of
		\param max_angle Maximum allowed angle in radians
		\param min_angle Minimum allowed angle in radians

		\returns
		True if vec's altitude is between min and max angle, false if it isn't. 
		
		\details
		vec is converted to spherical coordinates to determine phi, which is then compared
		to max and min angle to calculate the result.
	
	*/
	inline bool AltitudeWithinRange(const std::array<float, 3>& vec, float max_angle, float min_angle) {
		float x = vec[0]; float y = vec[1]; float z = vec[2];
		float r = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));
		float phi = acos(z / r);

		return (phi >= min_angle && phi <= max_angle);
	}

	/*!
		Equally distribute points on a sphere using Fibbonacci 
		
		\param num_points Number of points to generate. The actual number of points generated
		will be lower based given field of view limits.
		\param upwards_fov Maximum altitude of generated points in degrees.
		\param downward_fov Minimum altitude of points in degrees.

		\details
		Implementation is based on
		https://stackoverflow.com/questions/9600801/evenly-distributing-n-points-on-a-sphere.

		\see AltitudeWithinRange to see how altitude is limited based on upward/downward fov.
	*/ 
	vector<std::array<float, 3>> FibbonacciDist(int num_points, float upwards_fov, float downward_fov) 
	{

		// Convert limits to radians
		const float upperlimit = ConvertToRadians(upwards_fov);
		const float lowerlimit = ConvertToRadians(downward_fov);

		// Calculate maximum and minimum altitude
		const float max_phi = static_cast<float>(M_PI_2) + lowerlimit;
		const float min_phi = static_cast<float>(M_PI_2) - upperlimit;

		const int n = num_points;
		std::vector<std::array<float, 3>> out_points;
		float offset = 2.0f / float(n);
		const float increment = static_cast<float>(M_PI)* (3.0f - sqrtf(5.0f));

		for (int i = 5; i < n + 5; i++) {
			float y = ((float(i) * offset) - 1.0f) - (offset / 0.2f);
			float r = sqrtf(1.0f - powf(y, 2));
			
			float phi = (float(i + 1) * increment);
			float x = cosf(phi) * r;
			float z = sinf(phi) * r;

			// Check for nans. These can sometimes occur at the beginning and end of the result set
			// due to imprecision.
			if (!std::isnan(x) && !std::isnan(y) && !std::isnan(z)) {
				auto new_point = std::array<float, 3>{x, y, z};
				Normalize(new_point);
				if (AltitudeWithinRange(new_point, max_phi, min_phi))
					out_points.emplace_back(new_point);
			}
		}
		return out_points;

	}

	vector<std::array<float, 3>> FibbonacciDistributePoints(int num_points, float upwards_fov, float downward_fov)
	{
		// Generate initial set of directions
		auto out_points = FibbonacciDist(num_points, upwards_fov, downward_fov);
		
		// Calculate a percentage of points discarded. 
		int points_removed = num_points - out_points.size();
		double percent_removed = static_cast<double>(points_removed) / static_cast<double>(num_points);

		// Calculate a new total based on percentage of points removed
		int new_out_points = (num_points / (1.0 - percent_removed));
		
		// Generate a set of points with the new total.
		out_points.clear();
		return FibbonacciDist(new_out_points, upwards_fov, downward_fov);
	}
}
