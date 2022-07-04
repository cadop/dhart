#include <vector>
#include <array>

#ifndef HF_SPATIAL_STRUCTURES
#define HF_SPATIAL_STRUCTURES

/*!
	\brief Perform human scale analysis on 3D environments.

	\remarks
	This is the base namespace for the DHARTAPI package. All future modules should remain in this
	namespace and this should act as a jump page for all major sections of documentation.

*/
namespace HF {
}

#define C_INTERFACE extern "C" __declspec(dllexport) int

/// <summary>
/// Convert a raw array from an external caller to an organized vector of points
/// </summary>
/// <param name="raw_array">Pointer to the external array</param>
/// <param name="size">The number of points stored in the raw array, equal to the total number of floats / 3</param>
/// <returns> A list of points</returns>

/*!
	\code
		// Requires #include "cinterface_utils.h"

		float vertices[] = { 34.1, 63.9, 16.5, 23.5, 85.7, 45.2, 12.0, 24.6, 99.4 };
		const int size = 9;

		std::vector<std::array<float, 3>> points = ConvertRawFloatArrayToPoints(vertices, size);
	\endcode
*/
std::vector<std::array<float, 3>> ConvertRawFloatArrayToPoints(const float* raw_array, int size);

/// <summary>
/// Convert a raw array from an external caller to an organized vector of points
/// </summary>
/// <param name="raw_array">Pointer to the external array</param>
/// <param name="size">The number of points stored in the raw array, equal to the total number of ints / 3</param>
/// <returns> A list of points</returns>

/*!
	\code
		// Requires #include "cinterface_utils.h"

		int raw_array = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
		const int size = 9;

		std::vector<std::array<int, 3>> points = ConvertRawIntArrayToPoints(raw_array, size);
	\endcode
*/
std::vector<std::array<int, 3>> ConvertRawIntArrayToPoints(const int* raw_array, int size);

/// <summary>
/// Delete a float vector that's pointed to by <paramref name="float_vector"/>
/// </summary>

/*!
	\code
		// Requires #include "cinterface_utils.h"

		std::vector<float>* vec = new std::vector<float>{ 3, 6, 9 };

		// operator delete called on vec within DestroyFloatVector
		if (DestroyFloatVector(vec)) {
			std::cout << "DestroyFloatVector was successful" << std::endl;
		}
		else {
			std::cout << "DestroyFloatVector unsuccessful" << std::endl;
		}
	\endcode
*/
C_INTERFACE DestroyFloatVector(std::vector<float>* float_vector);


/*! 
	\brief Delete a vector of integers.
	
	\param int_vector A pointer to a vector of integers to delete.

	\returns `HF_STATUS.OK` on completion.
*/
C_INTERFACE DestroyIntVector(std::vector<int>* int_vector);

C_INTERFACE DestroyCharArray(char* char_array);

/// <summary>
/// Delete some object pointed to by ptr
/// </summary>
/// @tparam T	datatype of ptr

/*!
	\code
		// Requires #include "cinterface_utils.h"

		std::vector<int>* vec = new std::vector<int>{ 6, 3, 1, 0, 5, 0 };

		DeleteRawPtr<std::vector<int>>(vec);

		// At this point here, we assume DeleteRawPtr was successful.
		// We set vec to nullptr to eliminate a dangling pointer.
		if (vec) {
			vec = nullptr;
		}
	\endcode
*/
template <typename T>
inline void DeleteRawPtr(T * ptr ) {
	if (ptr)
		delete ptr;
}

#endif
