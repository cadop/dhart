#pragma once
#include <vector>
#include <array>
namespace HF {
	namespace RayTracer {
		struct FullRayRequest {
			float x, y, z;			// X Y and Z origin points
			float dx, dy, dz;		// X, Y and Z direction to fire the ray in
			bool was_fired = false; // Whether this ray was fired or not
			float distance = -1;	// Before firing: the maximum distance of the ray. After firing: distance to the hitpoint or -9999 on miss
			int mesh_id = -1;		// The ID of the hit mesh
			
			FullRayRequest(float x, float y, float z, float dx, float dy, float dz, float Distance = -1);
			FullRayRequest() {};
			
			/// <summary>
			/// Tells if the rayrequest it or not. If true, the hit was successful, false otherwise.
			/// Rays that weren't fired yet are considered as not hitting.
			/// </summary>
			bool didHit();
		};

		/// <summary>
		/// Contains no information about the ray itself, 
		/// </summary>
		struct HitInfo {
		
		};
		/// <summary>
		/// A ray request, that depends on RayRequestLiteHolder
		/// </summary>
		struct RayRequestLite {
			const std::array<float, 3> * origin; // A pointer to an origin in the collection's origins array
			const std::array<float, 3> * direction; // A pointer to a direction in the collection's directions array
			float distance;
			int meshid;

			std::array<float, 3> Direction() const;
			std::array<float, 3> Origin() const;

			void RecordResult(float dist, int meshid);
		};

		/// <summary>
		/// Holds a set of ray requests, reusing memory where possible
		/// </summary>
		class RayRequestCollection {
			std::vector<RayRequestLite> requests;
			std::vector<std::array<float, 3>> directions; // The vector of directions
			std::vector<std::array<float, 3>> positions;  // The vector of positions

			RayRequestCollection(std::vector<std::array<float, 3>> positions, std::vector<std::array<float, 3>> directions);
		};
	}
}
// Standardized hit struct
