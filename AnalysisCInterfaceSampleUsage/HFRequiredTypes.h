/*!
	\file		HFRequiredTypes.h
	\brief		Header file for providing complete types from the HF namespace

	\author		Gem Aludino
	\date		29 Jul 2020
*/

#ifndef HF_REQUIRED_TYPES_H
#define HF_REQUIRED_TYPES_H

#include <vector>

namespace HF::SpatialStructures {
	enum NODE_TYPE {
		GRAPH = 0,		///< This node is a graph node.
		POI = 1,		///< POI is 'point of interest'
		OTHER = 2		///< This node doesn't belong in any other category. 
	};

	struct Node;
	struct PathMember;
	struct Path;
	struct CSRPtrs;
};

namespace HF::Exceptions {
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
		NO_COST = -12,			///< There is no cost with the given name in the given graph
		NOT_COMPRESSED = -13,	///< Graph wasn't compressed!
	};
};

struct HF::SpatialStructures::Node {
	float x, y, z;	///< Cartesian coordinates x, y, z
	short type = HF::SpatialStructures::NODE_TYPE::GRAPH;
	int id;			///< Node identifier
};

struct HF::SpatialStructures::PathMember {
	float cost;		///< Cost of traversing to the next path member in the path.
	int node;		///< ID of the node this represents in the graph.
};

struct HF::SpatialStructures::Path {
	std::vector<PathMember> members; ///< Ordered array of PathMembers that comprise the path
};

struct HF::SpatialStructures::CSRPtrs {
	int nnz;	///< Number of non-zeros contained by the CSR
	int rows;	///< Number of rows in this CSR.
	int cols;	///< Number of columns in this CSR.

	float* data;			///< Stores the coefficient values of the non-zeros.
	int* outer_indices;		///< Stores for each column (resp. row) the index of the first non-zero in the previous two arrays.
	int* inner_indices;		///< Stores the row (resp. column) indices of the non-zeros.
};

#endif /* HF_REQUIRED_TYPES_H */