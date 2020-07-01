///
/// \file		HFExceptions.h
/// \brief		Contains definitions for the <see cref="HF::Exceptions">Exceptions</cref> namespace
///
///	\author		TBA
///	\date		26 Jun 2020

#pragma once

#include <exception>
#include <cstring>

/*!
	\brief Custom exceptions and error codes used interally by HumanFactors.

	\remarks
	These were mostly created as needed for the CInterface. If a certain error code needs to be returned,
	it should have a unique exception unless one can already be found in the std library or we want to
	differentiate between the exceptions we thrown and the exceptions the standard library throws. 
*/
namespace HF::Exceptions{
	/*! 
		\brief A set of error codes standard throughout every HumanFactors codebase.

		\remarks 
		Consider adding new error codes if the specific code you require can't be
		found here. Just make sure to update C# and Python interface's enums as
		well. 
	*/
	enum HF_STATUS {
		OK = 1,					///< Operation was successful 

		// Special Codes
		NOT_IMPLEMENTED = -54,	///< This function hasn't been implemented yet

		// Error Codes 
		GENERIC_ERROR = 0,		///< Not sure what happened here (If this gets thrown, either fix it or give it a status code!).
		NOT_FOUND = -1,			///< The path given did not lead to any file.
		INVALID_OBJ = -2,		///< The given path did not point to a valid obj file.
		NO_GRAPH = -3,			///< This requires a valid graph in the DB to execute successfully. 
		INVALID_COST = -4,		///< The given cost name does not exist in the database.
		MISSING_DEPEND = -5,	///< A dependency for this object is missing. 
		OUT_OF_MEMORY = -6,		///< Ran out of memory during the last operation.
		MALFORMED_DB = -7,		///< The database exists, but is in some kind of error state.
		DB_BUSY = -8,			///< The database is busy (is there some external connection?).
		INVALID_PTR = -9,		///< One or more of the given pointers didn't lead to anything.
		OUT_OF_RANGE = -10,     ///< Tried to reference something not in the given container.
		NO_PATH = -11,			///< There is no path between the start and end points.
	};

	/*! \brief Thrown when desired file is not found */
	struct FileNotFound : public std::exception
	{
		const char* what() const throw ()
		{
			return "Couldn't find the given file!";
		}
	};

	/*! \brief The OBJ file was not valid. */
	struct InvalidOBJ : public std::exception
	{
		const char* what() const throw ()
		{
			return "The obj given couldn't be read!";
		}
	};

	/*! \brief Thrown when a dependency is missing such as Embree. */
	struct MissingDependency : public std::exception
	{
		const char* what() const throw ()
		{
			return "C++ Exception";
		}
	};
}

