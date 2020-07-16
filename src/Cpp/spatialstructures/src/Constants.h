///
///	\file		Constants.h 
///	\brief		Contains definitions for the <see cref="HF::SpatialStructures">HF::SpatialStructures</see> namespace
///
/// \author	TBA 
/// \date	06 Jun 2020

#pragma once

namespace HF {

	/*! \brief Contains standard fundamental data structures for representing space used throughout HumanFactors. 
	
	\remarks
	The datatypes in the SpatialStructures Namespace are used throughout the HumanFactors library. 
	For example, the HF::GraphGenerator and HF::VisibilityGraph both
	produce a Graph	as output, allowing for the code to manage the Graph's internal CSR to be centralized
	in a single location.
	*/
	namespace SpatialStructures {

		/*!
			\brief Minimum value that can be represented in humanfactors.
			
			\details
			All rounding operations in HF::SpatialStructures will use this constant
			in rounding operations.
		*/

		// precision to use for rounding values
		constexpr float ROUNDING_PRECISION = 0.0001f; // Used to convert back to original value
		constexpr float FLOAT_PRECISION = 10000.0f;  // Used to convert to a given precision (avoids division)
		constexpr float GROUND_OFFSET = 0.001f; // Offset to be used for offsetting from a polygon when performing checks

		/// <summary> Round a a float to the nearest precision defined globally. The global values
		/// can be overridden with optional parameters p and r.  If r is 0.01, p must be 100.0, meaning
		/// there should be one more 0 between the decimal. </summary>
		/// \param f The desired float to round. 
		/// \param p The override of FLOAT_PRECISION (e.g., 10000.0f)
		/// \param r The override of ROUNDING_PRECISION (e.g., 0.0001f)
		/// <returns> f rounded to the nearest rounding_precision. </returns>
		/*!
			\code
				// be sure to #include "Constants.h" std::round from the cmath library is used in
				// roundhf. Precision is to the nearest ten-thousandth

				const float my_pi = 3.141592653589793238462643383279502884197169399375105820974944592307816406286;
				float rounded = HF::SpatialStructures::roundhf(my_pi);	// 	rounded == 3.1416
			\endcode
		*/
		
		float roundhf(float f, float p = FLOAT_PRECISION, float r = ROUNDING_PRECISION);
	}
}
