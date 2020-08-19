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
		constexpr float ROUNDING_PRECISION = 0.0001f; // Used to convert back to original value
		constexpr float FLOAT_PRECISION = 10000.0f;  ///< Used to convert to a given precision (avoids division)
		constexpr float GROUND_OFFSET = 0.001f; ///< Offset to be used for offsetting from a polygon when performing checks

		/// <summary> Round a float to the nearest precision defined globally. The global values
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

		/// <summary> Truncate a float to the nearest precision defined globally. The global values
		/// can be overridden with optional parameters p and r.  If r is 0.01, p must be 100.0, meaning
		/// there should be one more 0 between the decimal. </summary>
		/// \param f The desired float to truncate. 
		/// \param p The unit precision to truncate (Default, 1000.0f)
		/// \param r The override of the conversion back (e.g., 0.001f)
		/// <returns> f rounded to the nearest rounding_precision. </returns>
		float trunchf(float f, float p = 1000.0, float r = 0.001);

		/*!
			\brief round a number to the nearest precision defined globally. The global values
				   can be overridden with optional parameters p and r.  If r is 0.01, p must be 100.0, meaning
				   there should be one more 0 between the decimal.

			\tparam numeric_type Type of number to round. Can be float, double, or long double.

			\param f The desired value to round.
			\param p The unit precision to round (e.g, 1000.0f)
			\param r The override of the conversion back (e.g., 0.001f)

			\returns F rounded to the specified precision.
		*/
		template <typename numeric_type>
		inline constexpr numeric_type roundhf_tmp(
			numeric_type f,
			numeric_type p,
			numeric_type r
		) {	return std::round(f * p) * r;	}

		/*! 
			\brief  Round a number to the nearest value to itself at a specific precision. 

			\tparam desired_type Type to use and be returned by this calculation. All values will be
								 converted to this type before any operations are performed.
			\tparam numeric_type Type of number to round. Can be float, double, or long double.

			\param f The number to round. 
			\param r Precision to round to I.E. 0.0001 will round up to the 4th decimal place

			\returns `f` rounded to the nearest point specified in r. 
		*/
		template <typename desired_type, typename numeric_type>
		inline constexpr desired_type roundhf_tmp(numeric_type f,	numeric_type p) {
			return roundhf(static_cast<desired_type>(f), DivideBy1<desired_type>(p), static_cast<desired_type>(p));
		}

		/*!
			\brief  Round a number to the global rounding precision.

			\tparam desired_type Type to use and be returned by this calculation .All values will be
			converted to this type before any operations are performed.
			\tparam numeric_type Type of number to truncate. Can be float, double, or long double.

			\param f The number to round.

			\returns `f` rounded at ROUNDING_PRECISION.
		*/		template <typename desired_type, typename numeric_type>
		inline constexpr desired_type roundhf_tmp(numeric_type f) {
			return roundhf(
				static_cast<desired_type>(f),
				DivideBy1<desired_type>(ROUNDING_PRECISION),
				static_cast<desired_type>(ROUNDING_PRECISION)
			);
		}

		/*! 
			\brief truncate a number to the nearest precision defined globally. The global values
				   can be overridden with optional parameters p and r.  If r is 0.01, p must be 100.0, meaning
				   there should be one more 0 between the decimal.

			\tparam numeric_type Type of number to truncate. Can be float, double, or long double.

			\param f The desired float to truncate. 
			\param p The unit precision to truncate (Default, 1000.0f)
			\param r The override of the conversion back (e.g., 0.001f)

			\returns f truncated to the nearest rounding_precision. 
		*/
		template <typename numeric_type>
		inline constexpr numeric_type trunchf_tmp(
			numeric_type f,
			numeric_type p,
			numeric_type r
		) {	return std::trunc(f * p) * r; }
		
		/*! 
			\brief  Truncate a number to the nearest value not greater than itself at a specific precision.

			\tparam desired_type Type to useand be returned by this calculation.All values will be
			converted to this type before any operations are performed.
			\tparam numeric_type Type of number to truncate. Can be float, double, or long double.

			\param f The number to truncate.
			\param r Precision to truncate at I.E. 0.0001 will discard all values past the 4th decimal place

			\returns `f` truncated at the point specified in r.
		*/
		template <typename desired_type, typename numeric_type>
		inline constexpr desired_type trunchf_tmp(numeric_type f, numeric_type p) {
			return trunchf(static_cast<desired_type>(f), DivideBy1<desired_type>(p), static_cast<desired_type>(p));
		}
		/*!
			\brief  Truncate a number to the nearest value not greater than itself at a specific precision.

			\tparam desired_type Type to useand be returned by this calculation.All values will be
			converted to this type before any operations are performed.
			\tparam numeric_type Type of number to truncate.Can be float, double, or long double.

			\param f The number to truncate.

			\returns `f` truncated at ROUNDING_PRECISION.
		*/
		template <typename desired_type, typename numeric_type>
		inline constexpr desired_type trunchf_tmp(numeric_type f) {
			return trunchf(
				static_cast<desired_type>(f),
				DivideBy1<desired_type>(ROUNDING_PRECISION),
				static_cast<desired_type>(ROUNDING_PRECISION)
			);
		}

		/*! 
			\brief Cast a value to the specific type and divide 1 by it.

			\tparam numeric_type Type of number to divide 1 by
			\tparam desired_type Type to cast to and return. 

			\param n Number to divide 1 by.
		
			\returns 1/n
		*/
		template<typename desired_type, typename numeric_type>
		inline constexpr desired_type DivideBy1(numeric_type n) {
			return static_cast<desired_type>(1) /static_cast<desired_type>(n);
		}

	}
}
