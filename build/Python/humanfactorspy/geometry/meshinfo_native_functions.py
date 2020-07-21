from ctypes import *
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
    mesh_info_ptr = c_void_p(0)
    error_code = HFPython.LoadOBJ(
        GetStringPtr(obj_file_path),
        len(obj_file_path),
        c_float(rotation[0]),
        c_float(rotation[1]),
        c_float(rotation[2]),
        byref(mesh_info_ptr),
    )

    if error_code == HF_STATUS.OK:
        pass
    elif error_code == HF_STATUS.INVALID_OBJ:
        raise InvalidOBJException
    elif error_code == HF_STATUS.NOT_FOUND:  # soon to be own exception
        raise FileNotFoundException
    elif error_code == HF_STATUS.GENERIC_ERROR:
        raise HFException

    return mesh_info_ptr


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
