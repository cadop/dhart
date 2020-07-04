///
///	\file		cost_algorithms.h
///	\brief		Contains implementation for the <see cref="HF::SpatialStructures::CostAlgorithms">HF::SpatialStructures::CostAlgorithms</cref> namespace
///
///	\author		TBA
///	\date		03 Jul 2020
///

#include "cost_algorithms.h"
#include "graph.h"

#include <iostream>

using HF::SpatialStructures::IntEdge;
using HF::SpatialStructures::Graph;

std::vector<IntEdge> HF::SpatialStructures::CostAlgorithms::CalculateCrossSlope(Graph& g) {
	std::vector<IntEdge> result;

	// Leave this for now. We will construct a CSR in here just to get started.
	//CSRPtrs csr = g.GetCSRPointers();

	//
	// Get rid of this when you are ready to test the graph
	CSRPtrs csr;
	csr.nnz = 12;
	csr.rows = 5;
	csr.cols = 5;
	csr.data = new float[12]{ 2, 1, 2, 1, 3, 1, 4, 4, 3, 1, 3, 3 };
	csr.inner_indices = new int[12]{ 1, 4, 0, 2, 4, 1, 3, 2, 4, 0, 1, 3 };
	csr.outer_indices = new int[5]{ 0, 2, 5, 7, 9 };
	// 
	//

	//
	// May not really need these but they're here anyway
	float* data_begin = csr.data;
	float* data_end = csr.data + csr.nnz;

	int* inner_indices_begin = csr.inner_indices;
	int* inner_indices_end = csr.inner_indices + csr.nnz;

	int* outer_indices_begin = csr.outer_indices; 
	int* outer_indices_end = csr.outer_indices + csr.rows;

	int pos = 0;		// this represents the current index in csr.data
						// also used by csr.inner_indices
						// always increments until we end up at last address
						// in csr.data

	int sub_pos = 0;	// this number represents what parent node id we are on
						// we increment this when we move on to the next subarray
	//
	//
	
	int parent_id = 0;
	int child_id = 0;

	for (int i = 0; i < csr.nnz; i++) {
		float* edge_data = csr.data + i;
		child_id = csr.inner_indices[i];
	
		int next_child_id = -1;
		if (i < csr.nnz - 1) {
			next_child_id = csr.inner_indices[i + 1];
		}

		std::cout << parent_id << " -> " << child_id << " " << *edge_data << std::endl;

		//
		// Here, we can use parent_id, child_id, and *(edge_data)
		// to do what we need with our CSR.
		//

		if (next_child_id <= child_id) {
			++parent_id;
		}
	}

	/*
		struct CSRPtrs {
			int nnz;			// count of non-zero elems
			int rows;			// row count of decompressed adjacency matrix
			int cols;			// column count of decompressed adjacency matrix
								// also count of blocks in data

			float *data;		// pointer to buffer of edge distances (weights)
			int *inner_indices;	// pointer to buffer of column indices
			int *outer_indices;	// pointer to buffer of subarray offsets (for data)
		};


		For a struct CSRPtrs csr --
			csr.nnz is:
				- count of non-zero elems
				- block count of csr.data

			csr.rows is:
				- count of rows in decompressed adjacency matrix

			csr.cols is:
				- count of cols in decompressed adjacency matrix

			csr.data is:
				- pointer to buffer of edge weights
				- valid addresses range from [csr.data, csr.data + csr.nnz)

			csr.inner_indices is:
				- pointer to buffer of destination column indices in decompressed matrix
				- valid address range from [csr.inner_indices, csr.inner_indices + csr.nnz)

			csr.outer_indices is:
				- pointer to buffer of indices that split csr.data into subarrays
				  such that csr.data == csr.data + csr.outer_indices[0],
				  and csr.data + csr.outer_indices[1] is the address 
				  of the next subarray within csr.data.
				  A subarray contains the edge data for a particular row
				  in the decompressed adjacency matrix.

		Example:
			csr.data = 
			{ 2, 1, 2, 1, 3, 1, 4, 4, 3, 1, 3, 3 };
			
			csr.inner_indices = 
			{ 1, 4, 0, 2, 4, 1, 3, 2, 4, 0, 1, 3 };

			csr.outer_indices =
			{ 0, 2, 5, 7, 9 };


					0	1	2	3	4
					-----------------
			0		0	2	0	0	1
			1		2	0	1	0	3
			2		0	1	0	4	0
			3		0	0	4	0	3
			4		1	3	0	3	0


			address of subarray #0 (edge data of parent node id == 0)
				csr.data + csr.outer_indices[0]

				all children of parent node id 0 are at:
				csr.inner_indices

			address of subarray #1 (edge data of parent node id == 1)
				csr.data + csr.outer_indices[1]

			address of subarray #2 (edge data of parent node id == 2)
				csr.data + csr.outer_indices[2]

			address of subarray #3 (edge data of parent node id == 3)
				csr.data + csr.outer_indices[3]

			address of subarray #4 (edge data of parent node id == 4)
				csr.data + csr.outer_indices[4]

			
			
	*/

	// get rid of these when you are ready to test the graph!
	delete csr.inner_indices;
	delete csr.outer_indices;
	delete csr.data;

	return result;
}