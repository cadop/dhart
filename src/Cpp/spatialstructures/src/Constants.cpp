///
///	\file		Constants.cpp
///	\brief		Contains implementation for the <see cref="HF::SpatialStructures">HF::SpatialStructures</see> namespace
///
/// \author	TBA 
/// \date	06 Jun 2020

#include <Constants.h>
#include <numeric>
#include <cstdlib>
#include <cmath>

float HF::SpatialStructures::roundhf(float f, float p, float r)
{
	return std::round(f * p) * r ;
}
