#include <cinterface_utils.h>
#include <HFExceptions.h>


std::vector<std::array<float, 3>> ConvertRawFloatArrayToPoints(const float* raw_array, int size) {
	std::vector<std::array<float, 3>> out_array(size);
	for (int i = 0; i < size; i++) {
		const int os = i * 3;
		out_array[i][0] = raw_array[os];
		out_array[i][1] = raw_array[os + 1];
		out_array[i][2] = raw_array[os + 2];
	}
	return out_array;
}

std::vector<std::array<int, 3>> ConvertRawIntArrayToPoints(const int* raw_array, int size) {
	std::vector<std::array<int, 3>> out_array(size);
	for (int i = 0; i < size; i++) {
		const int os = i * 3;
		out_array[i][0] = raw_array[os];
		out_array[i][1] = raw_array[os + 1];
		out_array[i][2] = raw_array[os + 2];
	}
	return out_array;
}

C_INTERFACE DestroyFloatVector(std::vector<float>* float_vector)
{
	DeleteRawPtr(float_vector);
	return HF::Exceptions::HF_STATUS::OK;
}

