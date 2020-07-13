///
/// \file		RayRequest.h
/// \brief		Contains definitions for the <see cref="HF::RayTracer">RayTracer</see> namespace
///
///	\author		TBA
///	\date		26 Jun 2020

#pragma once
#include <vector>
#include <array>
namespace HF {
	namespace RayTracer {
		struct FullRayRequest {
			float x, y, z;			///< X, Y, and Z origin points
			float dx, dy, dz;		///< X, Y and Z direction to fire the ray in
			bool was_fired = false; ///< Whether this ray was fired or not
			float distance = -1;	///< Before firing: the maximum distance of the ray. After firing: distance to the hitpoint or -9999 on miss
			int mesh_id = -1;		///< The ID of the hit mesh

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

			/*!
				\code
					// Requires #include "RayRequest.h"

					// For brevity
					using HF::RayTracer::FullRayRequest;

					// Prepare FullRayRequest's parameters
					const float x_in = 0.0;
					const float y_in = 0.0;
					const float z_in = 0.0;
					const float dx_in = 1.0;
					const float dy_in = 1.0;
					const float dz_in = 2.0;
					const float distance_in = 10.0;

					// Create the FullRayRequest
					FullRayRequest request(x_in, y_in, z_in, dx_in, dy_in, dz_in, distance_in);
				\endcode
			*/
			FullRayRequest(float x, float y, float z,
				float dx, float dy, float dz,
				float Distance = -1);

			/// <summary>
			/// Default constructor, empty code block.
			/// </summary>

			/*!
				\code
					// Requires #include "RayRequest.h"

					// For brevity
					using HF::RayTracer::FullRayRequest;

					// Create the FullRayRequest (no arguments)
					FullRayRequest request;
				\endcode
			*/
			FullRayRequest() {};

			/// <summary>
			/// Tells if the rayrequest it or not. If true, the hit was successful, false otherwise.
			/// Rays that weren't fired yet are considered as not hitting.
			/// </summary>

			/*!
				\code
					// Requires #include "RayRequest.h"

					// For brevity
					using HF::RayTracer::FullRayRequest;

					// Prepare FullRayRequest's parameters
					const float x_in = 0.0;
					const float y_in = 0.0;
					const float z_in = 0.0;
					const float dx_in = 1.0;
					const float dy_in = 1.0;
					const float dz_in = 2.0;
					const float distance_in = 10.0;

					// Create the FullRayRequest
					FullRayRequest request(x_in, y_in, z_in, dx_in, dy_in, dz_in, distance_in);

					// Use didHit
					if (request.didHit()) {
						std::cout << "Hit" << std::endl;
					}
					else {
						std::cout << "Miss" << std::endl;
					}
				\endcode

				`>>>Miss`\n
			*/
			bool didHit();
		};
	}
}
// Standardized hit struct
