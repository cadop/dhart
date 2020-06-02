#include <Constants.h>
#include <numeric>
#include <cstdlib>
#include <cmath>

float HF::SpatialStructures::roundhf(float f)
{
	return std::round(f / ROUNDING_PRECISION) * ROUNDING_PRECISION ;
}
