/*!
	\file		HFExceptionStrings.cpp
	\brief		Source file for converting HF::Exception::HF_STATUS members to strings

	\author		Gem Aludino
	\date		24 Jul 2020
*/

#include "HFRequiredTypes.h"
#include "HFExceptionStrings.h"

/*!
	\brief		Convert a HF::Exceptions::HF_STATUS to a string, for easy interpretation of an error code

	\param		status_code		Member of HF::Exceptions::HF_STATUS

	\return		A string representing status_code, otherwise "(invalid status code)"
*/
std::string HF::Exceptions::code_to_str(int status_code) {
	std::string result = "";

	switch (status_code) {
	case HF::Exceptions::OK:
		result = "[OK]";
		break;
	case HF::Exceptions::NOT_IMPLEMENTED:
		result = "[functionality not implemented]";
		break;
	case HF::Exceptions::NOT_FOUND:
		result = "[file not found]";
		break;
	case HF::Exceptions::INVALID_OBJ:
		result = "[path does not refer to a valid .obj file]";
		break;
	case HF::Exceptions::NO_GRAPH:
		result = "[requires a valid graph in the database to execute successfully]";
		break;
	case HF::Exceptions::INVALID_COST:
		result = "[cost name provided does not exist in the database]";
		break;
	case HF::Exceptions::MISSING_DEPEND:
		result = "[a dependency for this object is missing]";
		break;
	case HF::Exceptions::OUT_OF_MEMORY:
		result = "[ran out of memory during the last operation]";
		break;
	case HF::Exceptions::MALFORMED_DB:
		result = "[the database exists, but is in an erroneous state]";
		break;
	case HF::Exceptions::DB_BUSY:
		result = "[the database is busy (is there an external connection?)";
		break;
	case HF::Exceptions::INVALID_PTR:
		result = "[one or more of the given pointers did not lead to a valid address]";
		break;
	case HF::Exceptions::OUT_OF_RANGE:
		result = "[attempted to reference a value that was not in the given container]";
		break;
	case HF::Exceptions::NO_PATH:
		result = "[no path between the starting and ending points]";
		break;
	default:
		result = "[invalid status code given]";
		break;
	}

	return result;
}
