#pragma once
namespace HF {

	namespace SpatialStructures {

		constexpr float ROUNDING_PRECISION = 0.0001f;
		/// <summary>
		/// Round a float using std::round
		/// </summary>
		/// <param name="f">The desired float to round</param>
		/// <returns>A rounded floating-point value using std::round and ROUNDING_PRECISION</returns>
		float roundhf(float f);
	}
}
