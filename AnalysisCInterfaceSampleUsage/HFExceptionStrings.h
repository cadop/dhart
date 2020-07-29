/*!
	\file		HFExceptionStrings.h
	\brief		Header file for converting HF::Exception::HF_STATUS members to strings

	\author		Gem Aludino
	\date		24 Jul 2020
*/

#ifndef HF_EXCEPTION_STRINGS_H
#define HF_EXCEPTION_STRINGS_H

#include <string>

namespace HF::Exceptions {
	std::string code_to_str(int status_code);
};

#endif /* HF_EXCEPTION_STRINGS_H */
