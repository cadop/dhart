from typing import Tuple, Union, List
from enum import IntEnum

from . import meshinfo_native_functions
from . import MeshInfo

__all__ = ['OBJGroupType','LoadOBJ']

class OBJGroupType(IntEnum):
    """ Different methods to seperate several meshes from a single OBJ """
    ONLY_FILE = 0  # Assign the same ID to all geometry in this file
    BY_GROUP = 1  # Assign different OBJ groups, different OBJs
    BY_MATERIAL = 2  # Seperate geometry based on material


def LoadOBJ(
    path: str,
    group_type: OBJGroupType = 0,
    rotation: Tuple[float, float, float] = (0, 0, 0),
) -> Union[MeshInfo, List[MeshInfo]]:
    """ Load an obj file from the given path. 
    
    Args:
        path: The path to a valid OBJ file.
        group_type: (NOT IMPLEMENTED) How IDs will be assigned to different
            parts of the mesh 
        rotation: Rotation in degrees to be performed on the OBJ after it is loaded. 
            Useful for converting models from Y-Up to Z-Up.
    
    Returns:
        MeshInfo: A new meshinfo object containing a the vertices and triangles
            for the obj in path. If group_type is not ONLY_FILE, a list of meshes
            may be returned if submeshes are found. 

    Raises:
        dhart.Exceptions.InvalidOBJException: The OBJ at path
            either did not exist or could not be loaded
        dhart.Exceptions.FileNotFoundException: No file exists
            at the given path.

    Example:
        Load plane.obj from a folder titled "Example Models" but fail because that file doesn't exist
        
        >>> import dhart
        >>> from dhart.geometry import LoadOBJ
        >>> obj = dhart.get_sample_model("not_real.obj")
        >>> MI = LoadOBJ(obj)
        Traceback (most recent call last):
        ...
        dhart.Exceptions.FileNotFoundException

        >>> import dhart
        >>> from dhart.geometry import LoadOBJ
        >>> obj = dhart.get_sample_model("plane.obj")
        >>> MI = LoadOBJ(obj)
        >>> MI 
        (EntireFile, 0)


    """

    mesh_ptr = meshinfo_native_functions.CreateOBJ(path, group_type, rotation)

    # Check return type. If multiple meshes were reutrned, then make a list
    if (not isinstance(mesh_ptr, list)):
        return MeshInfo(mesh_ptr)
    else:
        return [MeshInfo(ptr) for ptr in mesh_ptr]