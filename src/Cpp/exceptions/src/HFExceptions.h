#pragma once
#include <exception>
#include <cstring>
namespace HF {
	namespace Exceptions {
		enum HF_STATUS {
			OK = 1,

			// Special Codes //
			NOT_IMPLEMENTED = -54, // This function hasn't been implemented yet!

			// Error Codes //
			GENERIC_ERROR = 0,		// Not sure what happened here (If this gets thrown, either fix it or give it a status code!)
			NOT_FOUND = -1,			// The path given did not lead to any file
			INVALID_OBJ = -2,		// The given path did not point to a valid obj file
			NO_GRAPH = -3,			// This requires a valid graph in the DB to execute successfully
			INVALID_COST = -4,		// The given cost name does not exist in the database
			MISSING_DEPEND = -5,	// A dependency for this object is missing.
			OUT_OF_MEMORY = -6,		// Ran out of memory during the last operation
			MALFORMED_DB = -7,		// The database exists, but is in some kind of error state
			DB_BUSY = -8,			// The database is busy (is there some external connection?)
			INVALID_PTR = -9,		// One or more of the given pointers didn't lead to anything
			OUT_OF_RANGE = -10,     // Tried to reference something not in the given container
			NO_PATH = -11,			// There is no path between the start and end points
		};

		struct FileNotFound : public std::exception
		{
			const char* what() const throw ()
			{
				return "Couldn't find the given file!";
			}
		};
		struct InvalidOBJ : public std::exception
		{
			const char* what() const throw ()
			{
				return "The obj given couldn't be read!";
			}
		};
		struct MissingDependency : public std::exception
		{
			const char* what() const throw ()
			{
				return "C++ Exception";
			}
		};
	}
}
