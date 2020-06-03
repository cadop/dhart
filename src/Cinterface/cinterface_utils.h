#include <vector>
#include <array>



#define C_INTERFACE extern "C" __declspec(dllexport) int
/// <summary>
/// Convert a raw array from an external caller to an organized vector of points
/// </summary>
/// <param name="raw_array">Pointer to the external array</param>
/// <param name="size">The number of points stored in the raw array, equal to the total number of floats / 3</param>
/// <returns> A list of points</returns>
std::vector<std::array<float, 3>> ConvertRawFloatArrayToPoints(const float* raw_array, int size);
std::vector<std::array<int, 3>> ConvertRawIntArrayToPoints(const int* raw_array, int size);

/// <summary>
/// Delete a float vector that's pointed to by <paramref name="float_vector"/>
/// </summary>
C_INTERFACE DestroyFloatVector(std::vector<float>* float_vector);

/// <summary>
/// Delete some object pointed to by ptr
/// </summary>
template <typename T>
void DeleteRawPtr(T * ptr ) {
	if (ptr)
		delete ptr;
}