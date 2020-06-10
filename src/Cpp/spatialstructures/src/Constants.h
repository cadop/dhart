///
/// \file		Constants.h
///	\brief		Header file for constants/procedures in the HF::SpatialStructures namespace
///
/// \author		TBA
/// \date		06 Jun 2020
///
#pragma once

namespace HF {

	namespace SpatialStructures {

		constexpr float ROUNDING_PRECISION = 0.0001f;
		/// <summary>
		/// Round a float using std::round
		/// </summary>
		/// <param name="f">The desired float to round</param>
		/// <returns>A rounded floating-point value using std::round and ROUNDING_PRECISION</returns>

		/// \code{.cpp}
		/// // std::round from the cmath library is used in roundhf.
		/// // Precision is to the nearest ten-thousandth
		/// const float my_pi = 3.141592653589793238462643383279502884197169399375105820974944592307816406286
		/// float rounded = HF::SpatialStructures::roundhf(my_pi);	// 	rounded == 3.1416	
		/// \endcode
		float roundhf(float f);
	}
}
