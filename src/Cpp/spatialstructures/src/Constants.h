/// \file Constants.h \brief Header file for constants/procedures in the HF::SpatialStructures namespace
///
/// \author TBA \date 06 Jun 2020
#pragma once

namespace HF {

	/*! \brief Contains standard fundamental data structures for representing space used throughout HumanFactors. 
	
	\remarks
	The datatypes in the SpatialStructures Namespace are used throughout the HumanFactors library. 
	For example, the HF::AnalysisMethods::GraphGenerator and HF::AnalysisMethods::VisibilityGraph both
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
		constexpr float ROUNDING_PRECISION = 0.0001f;

		/// <summary> Round a a float to the nearest ROUNDING_PRECISION. </summary>
		/// <param name="f"> The desired float to round. </param>
		/// <returns> f rounded to the nearest rounding_precision. </returns>
		/*!
			\code
				// be sure to #include "Constants.h" std::round from the cmath library is used in
				// roundhf. Precision is to the nearest ten-thousandth

				const float my_pi = 3.141592653589793238462643383279502884197169399375105820974944592307816406286;
				float rounded = HF::SpatialStructures::roundhf(my_pi);	// 	rounded == 3.1416
			\endcode
		*/
		float roundhf(float f);
	}
}
