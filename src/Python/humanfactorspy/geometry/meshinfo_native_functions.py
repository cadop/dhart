from ctypes import *
import ctypes
from humanfactorspy.Exceptions import *
from humanfactorspy.common_native_functions import (
    getDLLHandle,
    ConvertPointsToArray,
    GetStringPtr,
)
from typing import *

HFPython = getDLLHandle()


def CreateOBJ(
    obj_file_path: str,
    group_type: int = 0,
    rotation: Tuple[float, float, float] = (0, 0, 0),
) -> c_void_p:
    """ Read an obj from the given file path in C++ and return a pointer to it.
    
    Raises:
        InvalidOBJException: OBJ fails to load from the given filepath
        FileNotFoundException: No file exists at the given filepath
    """
    # Setup output parameters
    mesh_info_ptr = c_void_p()
    num_meshes = c_int(0)

    # Call the function and capture the error code
    error_code = HFPython.LoadOBJ(
        GetStringPtr(obj_file_path),
        0,
        c_float(rotation[0]),
        c_float(rotation[1]),
        c_float(rotation[2]),
        byref(mesh_info_ptr),
        byref(num_meshes)
    )

    # Check ErrorCode
    if error_code == HF_STATUS.OK:
        pass
    elif error_code == HF_STATUS.INVALID_OBJ:
        raise InvalidOBJException
    elif error_code == HF_STATUS.NOT_FOUND:  
        raise FileNotFoundException
    elif error_code == HF_STATUS.GENERIC_ERROR:
        raise HFException

def CreateOBJ(
    obj_file_path: str,
    group_type: int = 0,
    rotation: Tuple[float, float, float] = (0, 0, 0)) -> c_void_p:
    """ Read an obj from the given file path in C++ and return a pointer to it.
    
    Raises:
        InvalidOBJException: OBJ fails to load from the given filepath
        FileNotFoundException: No file exists at the given filepath
    """
    # Setup output parameters
    mesh_info_ptr = c_void_p()
    num_meshes = c_int(0)

    # Call the function and capture the error code
    error_code = HFPython.LoadOBJ(
        GetStringPtr(obj_file_path),
        c_int(group_type),
        c_float(rotation[0]),
        c_float(rotation[1]),
        c_float(rotation[2]),
        byref(mesh_info_ptr),
        byref(num_meshes)
    )

    # Check ErrorCode
    if error_code == HF_STATUS.OK:
        pass
    elif error_code == HF_STATUS.INVALID_OBJ:
        raise InvalidOBJException
    elif error_code == HF_STATUS.NOT_FOUND:  
        raise FileNotFoundException
    elif error_code == HF_STATUS.GENERIC_ERROR:
        raise HFException

    # Cast to a pointer of c_void_p, then insert all values into
    # a python list
    mi_array = ctypes.cast(mesh_info_ptr, ctypes.POINTER(c_void_p))
    return_ptrs = [c_void_p(mi_array[i]) for i in range(0, num_meshes.value)]

    # Clean up the pointer array now that we've gotten what we need
    # from it
    HFPython.DestroyMeshInfoPtrArray(mesh_info_ptr)

    # Return only one value if only one mesh was returned
    return return_ptrs[0] if num_meshes.value == 1 else return_ptrs

def CreateMesh(
    indices: List[int], vertices: List[float], name: str, id: int
) -> c_void_p:
    """ Pass mesh data to C++ as a list of indices and vertices
    
    Args:
        indices: A list of indices with each 3 representing a triangle. Must be a multiple of 3
        vertices: A list of vertices, with each 3 floats representing a unique vertex
        name: The name of the mesh internally
        id: The mesh's ID

    Returns:
        A pointer to a valid MeshInfo object in c++
    """
    num_indices = len(indices)
    num_vertices = len(vertices)
    index_array_type = c_int * num_indices
    vertex_array_type = c_float * num_vertices
    index_array = index_array_type(*indices)
    vertex_array = vertex_array_type(*vertices)

    mesh_info_ptr = c_void_p(0)
    error_code = HFPython.StoreMesh(
        byref(mesh_info_ptr),
        index_array,
        c_int(num_indices),
        vertex_array,
        c_int(num_vertices),
        GetStringPtr(name),
        c_int(id),
    )

    if error_code == HF_STATUS.OK:
        pass
    elif error_code == HF_STATUS.INVALID_OBJ:
        raise InvalidOBJException
    elif error_code == HF_STATUS.NOT_FOUND:
        raise InvalidOBJException
    elif error_code == HF_STATUS.GENERIC_ERROR:
        raise HFException

    return mesh_info_ptr


def C_RotateMesh(mesh_ptr: c_void_p, rotation: Tuple[float, float, float]) -> None:
    HFPython.RotateMesh(mesh_ptr, c_float(rotation[0]), c_float(rotation[1]), c_float(rotation[2]))
    return


def DestroyMeshInfo(mesh_info_ptr: c_void_p):
    """ Call the destructor for MeshInfo """
    HFPython.DestroyMeshInfo(mesh_info_ptr)
