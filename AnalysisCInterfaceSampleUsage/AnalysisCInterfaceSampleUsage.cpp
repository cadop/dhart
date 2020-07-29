/*!
	\file		AnalysisCInterfaceTesting.cpp
	\brief		Source file for testing functionality of (purely) the C Interface

	\author		Gem Aludino
	\date		24 Jul 2020
*/

#include <iostream>
#include <string>

#include "HFExceptionStrings.h"
#include "HFRequiredTypes.h"

///
///	Required for calling C Interface functions
///
#include "spatialstructures_C.h"
#include "view_analysis_C.h"
#include "visibility_graph_C.h"
#include "raytracer_C.h"
#include "pathfinder_C.h"
#include "objloader_C.h"
#include "analysis_C.h"

/*!
	\namespace	cit			cit, short for C Interface Tests
	\brief		Testing		of C Interface, without direct access to C++ backend
*/
namespace cit {
	const std::string relative_example_models_path = ".\\Example Models\\";

	void raycasting(void);
	void generate_graph(void);
	void get_csr_graph(void);
	void visualize_graph(void);
	void basic_search_graph(void);
	void visualize_path(void);
	void path_plan_costs(void);
	void calculating_spatial_view(void);
	void create_visibility_graph(void);

	const std::string to_obj_path(const std::string& filename);

	std::ostream& operator<<(std::ostream& os, const HF::SpatialStructures::CSRPtrs& csr);
};

using HF::Exceptions::HF_STATUS;
using HF::Exceptions::code_to_str;

/*!
	\brief		Program execution begins here

	\param	argc	Argument count
	\param	argv	Argument values, command line arguments

	\return	0 on success, else error code
*/
int main(int argc, const char* argv[]) {
	cit::raycasting();
	cit::generate_graph();
	cit::get_csr_graph();
	cit::visualize_graph();
	cit::basic_search_graph();
	cit::visualize_path();
	cit::path_plan_costs();
	cit::calculating_spatial_view();
	cit::create_visibility_graph();

	return 0;
}


/*!
	\brief	Operator overload for <<, writes string representation of a CSRPtrs struct to an output stream

	\param	os	reference to an output stream, i.e. std::cout, or a file
	\param	csr	reference to a HF::SpatialStructures::CSRPtrs, the CSR to stream

	\return	reference to mutated output stream, with data from parameter csr
*/
std::ostream& cit::operator<<(std::ostream& os, const HF::SpatialStructures::CSRPtrs& csr) {
	///
	/// CSR traversal
	///

	float* curr = csr.data;					// address of current position within edge data
	float* data_end = csr.data + csr.nnz;	// address of one-past the last element within edge data

	int* inner = csr.inner_indices;			// address of current position within child node id buffer (column value)
	int row = 0;							// value denoting the current parent node id (row number)

	while (curr < data_end) {
		// While edge data remains...

		// Note the current position within the edge data buffer.
		// This is the address that denotes the beginning of a row.
		float* row_begin = curr;

		// If we are at the last row index,
		// row_end is data_end -- else,
		// row_end is the address of the next row's (row + 1) initial value.
		float* row_end =
			(row == csr.rows - 1)
			? data_end : csr.data + csr.outer_indices[row + 1];

		while (curr < row_end) {
			// While curr is not at the end of the current row...

			// row is the parent node id
			// *inner is the child node id
			// *curr is the edge value between parent node id and child node id
			os << "(" << row << ", " << *inner << ")"
				<< "\t\t" << *curr << std::endl;

			++inner;	// advance the address of inner (child node id buffer)
			++curr;		// advance the address of curr (edge data buffer)
		}

		++row;	// advance the row value (parent node id)
	}

	return os;
}

void cit::raycasting() {

}

void cit::generate_graph() {

}

void cit::get_csr_graph() {

}

void cit::visualize_graph() {

}

void cit::basic_search_graph() {

}

void cit::visualize_path() {

}

void cit::path_plan_costs() {

}

void cit::calculating_spatial_view() {

}

void cit::create_visibility_graph() {

}

/*!
	\brief	Takes string filename and appends it to the relative path
			of the Example Models directory, where example .obj files are stored

	\param	filename	The filename of the desired .obj file, relative to the Example Models directory

	\return		A string that represents the filepath of an file named filename,
				relative to this project file's path

	\code
		const std::string my_obj_file = cit::to_obj_path("big_teapot.obj");
		// my_obj_file reads as
		//     "..\\..Cpp\\tests\\Example Models\\big_teapot.obj"
	\endcode
*/
const std::string cit::to_obj_path(const std::string& filename) {
	return cit::relative_example_models_path + filename;
}
