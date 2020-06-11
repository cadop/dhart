///
/// \file		Constants.cpp
///	\brief		Source file for constants/procedures in the HF::SpatialStructures namespace
///
/// \author		TBA
/// \date		06 Jun 2020
///
#include <Constants.h>
#include <numeric>
#include <cstdlib>
#include <cmath>

float HF::SpatialStructures::roundhf(float f) {
	return std::round(f / ROUNDING_PRECISION) * ROUNDING_PRECISION ;
}
