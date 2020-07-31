from ctypes import *
from humanfactorspy.Exceptions import *
from typing import *

from humanfactorspy.common_native_functions import (
    getDLLHandle,
    ConvertPointsToArray,
    ConvertIntsToArray,
    convert_strings_to_array,
    GetStringPtr
)

HFPython = getDLLHandle()


def SizeOfNodeVector(node_vector_ptr: c_void_p) -> int:
    """ Get the size of a node vector """
    size = c_int(0)
    HFPython.GetSizeOfNodeVector(node_vector_ptr, byref(size))
    return size.value


def GetEdgesForNode(graph_ptr: c_void_p, node_ptr: c_void_p) -> Tuple[c_void_p, c_void_p]:
    """ *** UNIMPLEMENTED *** 
    
    Get a list of nodes from a graph that belong to the specified node
    """
    pass


def C_AggregateEdgeCosts(
        graph_ptr: c_void_p,
        aggregate_type: int,
        directed: bool,
        cost_type: str) -> c_void_p:
    """
    Aggregates edge costs

    Notes
    -----

    Calls `C_INTERFACE AggregateCosts`

    """

    # Pointers to store results
    vector_ptr = c_void_p(0)
    data_ptr = c_void_p(0)
    cost_type_ptr = GetStringPtr(cost_type)

    # Call to C interface
    error_code = HFPython.AggregateCosts(
        graph_ptr,
        c_int(aggregate_type),
        c_bool(directed),
        cost_type_ptr,
        byref(vector_ptr),
        byref(data_ptr)
    )

    # Check error code
    if error_code == HF_STATUS.OK:
        # Return result pointers
        return vector_ptr, data_ptr
    elif error_code == HF_STATUS.NO_COST:
        # Throw if they try to use a cost type that doesn't exist
        raise KeyError(f"Tried to aggregate the edges of non existant "
                       + f"cost type {cost_type}")
    else:
        assert(False)  # Never should get here, this is a programmer error


def GetNodesFromGraph(graph_ptr: c_void_p) -> Tuple[c_void_p, c_void_p]:
    """ Get a list of nodes from from the graph pointed to by graph_ptr

    Returns:
        Tuple[c_void_p, c_void_p] :a pointer to a vector of nodes then a pointer to 
            said vector's underlying data.
    """
    vector_ptr = c_void_p(0)
    data_ptr = c_void_p(0)
    HFPython.GetAllNodesFromGraph(graph_ptr, byref(vector_ptr), byref(data_ptr))

    return vector_ptr, data_ptr


def C_CreateGraph(nodes: Union[List[Tuple[float, float, float]], None]) -> c_void_p:
    """ Create a new graph in C++
    
    Args:
        nodes: A list of nodes to pre-insert into the graph or nothing
    Returns:
        c_void_p: A pointer to the underlying graph object in C++
    """
    if nodes:
        num_nodes = len(nodes)
        node_float_ptr = ConvertPointsToArray(nodes)
    else:
        node_float_ptr = c_void_p()
        num_nodes = 0

    graph_ptr = c_void_p()
    HFPython.CreateGraph(node_float_ptr, c_int(num_nodes), byref(graph_ptr))

    return graph_ptr


def C_AddEdgeFromNodes(
    graph_ptr: c_void_p,
    parent: Tuple[float, float, float],
    child: Tuple[float, float, float],
    score: float,
    cost_type: str,
    ) -> None:
    """ Add a new edge to the graph """

    # Convert to types usable in C
    parent_ptr = ConvertPointsToArray(parent)
    child_ptr = ConvertPointsToArray(child)
    str_ptr = GetStringPtr(cost_type)

    # Call to native code and capture the error code
    error_code = HFPython.AddEdgeFromNodes(
        graph_ptr,
        parent_ptr,
        child_ptr,
        c_float(score),
        cost_type
    )

    # Throw if the error code demands it
    if error_code == HF_STATUS.OK:
        return
    elif error_code == HF_STATUS.NOT_COMPRESSED:
        raise LogicError(
            message="The graph wasn't compressed before adding an alternate edge ")
    elif error_code == HF_STATUS.OUT_OF_RANGE:
        raise InvalidCostOperation(
            f"Tried to add an edge from {parent} to child to alternate cost"
            + f"type {cost_type} without first creating an edge between them"
            + "in the graph's default cost set.")
    else:
        print("Unexpected error code: " + error_code)
        assert(
            False,
            "There's some error that's not being handled either in C++ or"
            + "in python.")


def C_AddEdgeFromNodeIDs(
    graph_ptr: c_void_p,
    parent_id: int,
    child_id: int,
    score: float,
    cost_type: str
) -> None:
    """
    Adds edge to graph from a node ID

    Returns:
        None

    """
    # Get a pointer to cost_type
    string_ptr = GetStringPtr(cost_type)

    # Try to add the edge to the graph
    error_code = HFPython.AddEdgeFromNodeIDs(
        graph_ptr,
        c_int(parent_id),
        c_int(child_id),
        c_float(score),
        string_ptr
    )

    # Check error code.
    if error_code == HF_STATUS.OK:
        # On success return
        return
    elif error_code == HF_STATUS.NOT_COMPRESSED:
        # Can't add alternate costs to an uncompressed graph
        raise LogicError(
            "Tried to add an alternate cost type to the graph before" +
            "compressing it")
    elif error_code == HF_STATUS.OUT_OF_RANGE:
        # Tried to add an edge to an alternate cost type
        # that didn't exist
        raise InvalidCostOperation(
            f"Tried to add an edge from {parent_id} to {child_id} to alternate"
            + " cost type {cost_type} without first creating an edge between"
            + "them in the graph's default cost set.")
    elif error_code == HF_STATUS.GENERIC_ERROR:
        print("Unexpected error code: " + error_code)
        assert(False)  # Something is happening in C++ that isn't being
        # handled by python, or should never happen at all


def C_GetCSRPtrs(
        graph_ptr: c_void_p,
        cost_type: str) -> Tuple[int, int, int, c_void_p, c_void_p, c_void_p]:
    """ Get the information necessary to map a numpy CSR to the C++ graph

    Parameters:

    graph_ptr : c_void_p
        a pointer to the graph object

    cost_type : c_char_p
        The cost type to use for constructing the CSR.

    Returns:
        int: Number of non-zeros for the csr
        int: Number of rows in the graph
        int: Number of columns in the graph
        c_void_p: Pointer to the data of the graph
        c_void_p: Pointer to the inner_indices of the graph
        c_void_p: Pointer to the outer_indices of the graph
    """

    # Define out variables. These will be updated when the native function
    # is called
    nnz = c_int(0)
    num_cols = c_int(0)
    num_rows = c_int(0)
    data_ptr = c_void_p(0)
    inner_indices_ptr = c_void_p(0)
    outer_indices_ptr = c_void_p(0)

    # Convert cost type to a c_string
    cost_type_ptr = GetStringPtr(cost_type)

    # Get the CSR pointers and capture the error code
    res = HFPython.GetCSRPointers(
        graph_ptr,
        byref(nnz),
        byref(num_rows),
        byref(num_cols),
        byref(data_ptr),
        byref(inner_indices_ptr),
        byref(outer_indices_ptr),
        cost_type_ptr,
    )

    # Check the error code to see if we need to throw
    if res == HF_STATUS.OK:
        # OK means that things executed successfully so return
        return (
            nnz.value,
            num_rows.value,
            num_cols.value,
            data_ptr,
            inner_indices_ptr,
            outer_indices_ptr,
        )
    elif res == HF_STATUS.NO_COST:
        # No cost indicates that the cost didn't exist
        raise KeyError(
            f"Tried to get costs of nonexistant edge cost type {cost_type}")
    else:
        # Anything else indicates an unexpected exception in C++
        # Check the C_Interface to see if there's some case that's not being
        # handled here or there.
        print("Unexpected error code: " + error_code)
        assert(False)


def C_GetNodeID(graph_ptr: c_void_p, node: Tuple[float, float, float]) -> int:
    """ Get the id of node for the graph at graph_ptr """
    return_int = c_int()
    node_ptr = ConvertPointsToArray(node)
    HFPython.GetNodeID(graph_ptr, node_ptr, byref(return_int))

    return return_int


def C_GetEdgeCost(
        graph_ptr: c_void_p,
        parent: int,
        child: int,
        cost_type: str):
    """ Get the cost of an edge in the graph

    Args:
        graph_ptr: pointer to the graph to get the cost from
        parent: parent of the edge
        child: child of the edge
        cost_type: cost type to get the edge cost from. If left as the empty
                   string, use the graph's default cost type.

    Returns:
        The cost from parent to child if the cost exists, otherwise returns
        -1 if the cost doesn't exist.
    """

    # Define a float to serve as our output variable
    out_cost = c_float(0)

    # Get a pointer to cost_type
    cost_type_ptr = GetStringPtr(cost_type)

    # Execute the function and caputre the error code. If the error code
    # is HF_STATUS.OK then out_cost will be updated with the cost of
    # parent to child
    error_code = HFPython.GetEdgeCost(
        graph_ptr,
        c_int(parent),
        c_int(child),
        cost_type_ptr,
        byref(out_cost)
    )

    # Check error code
    if error_code == HF_STATUS.OK:
        # Return if the function executed correctly
        return out_cost.value
    elif error_code == HF_STATUS.NO_COST:
        # Throw if the cost doeesn't exist
        raise KeyError(
            f"Tried to get the cost of non-existant cost type: {cost_type}")
    else:
        # Indicates programmer error either here or in the cinterface
        print("Unexpected error code: " + error_code)
        assert(False)


def C_Compress(graph_ptr: c_void_p) -> None:
    HFPython.Compress(graph_ptr)


def C_ClearGraph(graph_ptr: c_void_p, cost_type: str = '') -> None:
    """
    Clear graph of a given cost type

    """
    cost_type_ptr = GetStringPtr(cost_type)

    error_code = HFPython.ClearGraph(graph_ptr, cost_type_ptr)

    if error_code == HF_STATUS.OK:
        return
    elif error_code == HF_STATUS.NO_COST:
        raise KeyError(f"Tried to clear non-existant cost {cost_type} from a"
                       + " graph")
    else:
        print("Unexpected error code: " + error_code)
        assert(False)  # There's some unhandled problem with C++


def C_NumNodes(graph_ptr: c_void_p) -> int:
    """ Get the number of nodes in the graph """

    out_size = c_int(0)

    # Calling this will update out_size with the number of nodes
    # in the graph
    error_code = HFPython.GetSizeOfGraph(graph_ptr, byref(out_size))

    # In docs, this only returns OK.
    assert(error_code == HF_STATUS.OK)

    return out_size.value

def C_CalculateAndStoreEnergyExpenditure(graph_ptr : c_void_p):
    error_code = HFPython.CalculateAndStoreEnergyExpenditure(graph_ptr)
    assert(error_code == HF_STATUS.OK, "CalculateAndStoreEnergyexpenditure  only should return OK. Something must have changed in the C++ code that hasn't been updated in python. ")

def C_CalculateAndStoreCrossSlope(graph_ptr : c_void_p):
    error_code = HFPython.CalculateAndStoreCrossSlope(graph_ptr)
    assert(error_code == HF_STATUS.OK, "CalculateAndStoreCrossSlope  only should return OK. Something must have changed in the C++ code that hasn't been updated in python. ")


def c_get_node_attributes(
    graph_ptr: c_void_p, attr: str, num_nodes: int
    ) -> Union[List[str], List[None]]:
    """ Get node attributes from a grpah in  C++

    Args:
        graph_ptr : Pointer to the graph to get attributes from.
        attr : Unique key of the attribute to get
        num_nodes : number of nodes in the graph

    Returns:
        A list of strings containing the score for every node in the graph
        ordered by ID. If the attribute could not be found in the graph, an
        empty list will be returned instead

    """

    # Define variables to meet preconditions
    attr_ptr = GetStringPtr(attr)
    out_score_type = c_char_p * num_nodes
    out_scores = out_score_type()
    out_scores_size = c_int(0)

    # Call into the function in C++. This will update
    # out_scores and out_scores_size
    error_code = HFPython.GetNodeAttributes(
        graph_ptr, attr_ptr, byref(out_scores), byref(out_scores_size)
    )

    # This function shouldn't return anything other than OK
    assert error_code == HF_STATUS.OK

    # Return an empty list if size is zero
    if out_scores_size.value == 0:
        return []

    # Read strings out of pointer and append them to our output
    out_strings = []
    for i in range(0, num_nodes):

        # .value of a charp reads the string
        score = out_scores[i].decode("utf-8")
        out_strings.append(score)

    # Deallocate the memory of the strings in C++
    HFPython.DeleteScoreArray(out_scores, out_scores_size)

    # Return output
    return out_strings


def c_add_node_attributes(
    graph_ptr: c_void_p, attr: str, ids: List[int], scores: List[str]
    ) -> None:
    """ Assign or update scores for nodes for a specific attribute

    Args:
        graph_ptr : Pointer to the graph to add scores to
        attr : Unique key of the attribute to add scores to. If this an 
        attribute with this keydoesn't exist then it will be created.
        ids : An array of ids of nodes to assign scores to 
        scores : An ordered array of scores where each score is to be assigned 
        to the id in ids at the same index.

    """

    # Convert to CTypes
    id_arr = ConvertIntsToArray(ids)
    score_arr = convert_strings_to_array(scores)
    attribute_ptr = GetStringPtr(attr)
    num_nodes = c_int(len(ids))

    # Call native function
    error_code = HFPython.AddNodeAttributes(
        graph_ptr, id_arr, attribute_ptr, score_arr, num_nodes
    )

    # Error code should only be OK
    assert error_code == HF_STATUS.OK

    return


def c_clear_node_attribute(graph_ptr: c_void_p, attr: str) -> None:
    """ Clear a node attribute and all of its scores from a graphj

    Args:
        graph_ptr : Graph to clear the attribute from.
        attr : Attribute to clear from the graph

    """

    # Convert score_ptr to a char array
    score_ptr = GetStringPtr(attr)

    # Call native C++ function
    error_code = HFPython.ClearAttributeType(graph_ptr, score_ptr)

    # Check Error Code
    assert error_code == HF_STATUS.OK  # This should only return OK

    return

### Destructors

def C_DeleteFloatArray(float_ptr: c_void_p):
    HFPython.DestroyFloatVector(float_ptr)

def DestroyGraph(graph_ptr: c_void_p):
    """ Call the destructor for a list of nodes """
    HFPython.DestroyGraph(graph_ptr)


def DestroyNodes(node_list_ptr: c_void_p):
    """ Call the destructor for a list of nodes """
    HFPython.DestroyNodes(node_list_ptr)
