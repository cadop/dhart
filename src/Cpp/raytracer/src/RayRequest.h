///
/// \file		RayRequest.h
/// \brief		Contains definitions for the <see cref="HF::RayTracer">RayTracer</cref> namespace
///
///	\author		TBA
///	\date		26 Jun 2020

#pragma once
#include <vector>
#include <array>
namespace HF {
	namespace RayTracer {
		struct FullRayRequest {
			float x, y, z;			///< X, Y, and Z origin points ///<
			float dx, dy, dz;		///< X, Y and Z direction to fire the ray in ///<
			bool was_fired = false; ///< Whether this ray was fired or not ///<
			float distance = -1;	///< Before firing: the maximum distance of the ray. After firing: distance to the hitpoint or -9999 on miss ///<
			int mesh_id = -1;		///< The ID of the hit mesh ///<
			
			/// <summary>
			/// Constructs a FullRayRequest from origin points (x, y, z) 
			/// and destination points (dx, dy, dz),
			/// as well as a starting distance for the ray.
			/// </summary>
			/// <param name="x">The x origin coordinate</param>
			/// <param name="y">The y origin coordinate</param>
			/// <param name="z">The z origin coordinate</param>
			/// <param name="dx">The x destination coordinate</param>
			/// <param name="dy">The y destination coordinate</param>
			/// <param name="dz">The z destination coordinate</param>
			/// <param name="Distance">The maximum distance of the ray (upon construction)</param>
			FullRayRequest(float x, float y, float z, 
						   float dx, float dy, float dz, 
						   float Distance = -1);

			/// <summary>
			/// Default constructor, empty code block.
			/// </summary>
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
		/// \deprecated Never used. Leftover from partially implemented experimental feature. This eventually became the RayResult
		/// struct seen in the C-interface. Moving the RayResult struct to the C interface was far more useful than what could be
		/// accomplished here, since it allowed the Cinterface and its clients to define their own RayRequest structs that could
		/// be filled through templates. 
		struct HitInfo {};

		/// <summary>
		/// A ray request, that depends on RayRequestLiteHolder
		/// </summary>
		/// \deprecated Unused. See HitInfo.
		struct RayRequestLite {
			const std::array<float, 3> * origin;	///< A pointer to an origin in the collection's origins array ///<
			const std::array<float, 3> * direction; ///< A pointer to a direction in the collection's directions array ///<
			float distance;							///< The ray's distance ///<
			int meshid;								///< The mesh ID of the ray ///<

			/// <summary>
			/// Retrieves a RayRequestLite's direction array, by value
			/// </summary>
			/// <returns>A std::array of type float, size 3, representing the desired direction for the RayRequestLite</returns>
			std::array<float, 3> Direction() const;
			
			/// <summary>
			/// Retrieves a RayRequestLite's origin array, by value
			/// </summary>
			/// <returns>A std::array of type float, size 3, representing the desired origin for the RayRequestLite</returns>
			std::array<float, 3> Origin() const;

			/// <summary>
			/// TODO summary
			/// </summary>
			/// <param name="dist">TODO name</param>
			/// <param name="meshid">TODO name</param>
			void RecordResult(float dist, int meshid);
		};

		/// <summary>
		/// Holds a set of ray requests, reusing memory where possible
		/// </summary>
		/// \deprecated Unused see HitInfo.
		class RayRequestCollection {
			std::vector<RayRequestLite> requests; ///< The vector of requests ///<
			std::vector<std::array<float, 3>> directions; ///< The vector of directions ///<
			std::vector<std::array<float, 3>> positions;  ///< The vector of positions ///<

			/// <summary>
			/// Constructs a RayRequestCollection from origin coordinates, and direction coordinates
			/// </summary>
			/// <param name="positions">An array of float, representing the origin coordinates of a ray</param>
			/// <param name="directions">An array of float, representing the direction coordinates of a ray</param>
			RayRequestCollection(std::vector<std::array<float, 3>> positions, std::vector<std::array<float, 3>> directions);
		};
	}
}
// Standardized hit struct
