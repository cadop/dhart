from ctypes import *
import ctypes
from dhart.Exceptions import *
from dhart.common_native_functions import (
    getDLLHandle,
    ConvertPointsToArray,
    GetStringPtr,
)
from typing import *

HFPython = getDLLHandle()

def CreateOBJ(
    obj_file_path: str,
    group_type: int = 0,
    rotation: Tuple[float, float, float] = (0, 0, 0)) -> Union[c_void_p, List[c_void_p]]:
    """Read an OBJ from a file on disk. 

    Args:
        obj_file_path (str): Path to the obj file on dik
        group_type (int, optional): Method of dividing the OBJ into submeshes. 
        rotation (Tuple[float, float, float], optional): Magnitude to rotate the mesh on the x,y,z axis

    Raises:
        InvalidOBJException: OBJ fails to load from the given filepath
        FileNotFoundException: No file exists at the given filepat

    Returns:
        Union[c_void_p, List[c_void_p]]: A pointer to one isntance of meshinfo if only one was created, or a list of pointers for each submesh in the OBJ at path 
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


    if error_code == HF_STATUS.INVALID_OBJ:
        raise InvalidOBJException
    elif error_code == HF_STATUS.NOT_FOUND:  
        raise FileNotFoundException

    assert(error_code == HF_STATUS.OK)

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


def C_GetMeshName(mesh_ptr : c_void_p) ->str:
    """Get the nme of a mesh from C++

    Args:
        mesh_ptr (c_void_p): A pointer to the meshinfo tor read from in native code

    Returns:
        str: The name of the mesh in C++ 
    """
    
    # Create a pointer for the output string, then call the C++ code
    # to update it
    out_str = c_void_p(0)
    HFPython.GetMeshName(mesh_ptr, byref(out_str))

    # Cast the string pointer to a char array
    str_ptr = c_char_p(out_str.value)
    
    # Decode the char array to get a python string
    out_name = str_ptr.value.decode("utf-8")

    # Deallocate the char array in C++
    HFPython.DestroyCharArray(out_str)

    return out_name

def C_GetMeshID(mesh_ptr : c_void_p) -> int:
    """ Get the ID of an instance of meshinfo in C++

    Args:
        mesh_ptr (c_void_p): Pointer to the mesh to get the ID from in C++

    Returns:
        int: [description]
    """
    mesh_id = c_int(0)
    HFPython.GetMeshID(mesh_ptr, byref(mesh_id))

    return mesh_id.value

def C_GetMeshVertsAndTris(mesh_ptr: c_void_p) -> Tuple[c_void_p, int, c_void_p, int]:
    """Get pointers to the vertex and triangle arrays of a meshinfo object

    Args:
        mesh_ptr (c_void_p): A pointer to the instance of meshinfo to get the vertex
        and triangle arrays of. 

    Returns:
        Tuple[c_void_p, int, c_void_p, int]: A pointer to the mesh's index array, the number of indicies contained within the mesh, a pointer to the vertex array of the mesh, and the number of vertices contained in the mesh respectively
    """
    index_ptr = c_void_p(0)
    num_triangles = c_int(0)
    vertex_ptr = c_void_p(0)
    num_vertices = c_int(0)

    HFPython.GetVertsAndTris(
        mesh_ptr, byref(index_ptr), byref(num_triangles), byref(vertex_ptr), byref(num_vertices)
    )

    return (index_ptr, num_triangles.value, vertex_ptr, num_vertices.value)



def DestroyMeshInfo(mesh_info_ptr: c_void_p):
    """ Call the destructor for MeshInfo """
    HFPython.DestroyMeshInfo(mesh_info_ptr)
