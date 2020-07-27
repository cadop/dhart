from typing import Tuple
from enum import IntEnum as Enum

from . import meshinfo_native_functions
from . import MeshInfo

__all__ = ['OBJGroupType','LoadOBJ']

class OBJGroupType(Enum):
    """ Different methods to seperate several meshes from a single OBJ """

    ONLY_FILE = 0  # Assign the same ID to all geometry in this file
    BY_GROUP = 1  # Assign different OBJ groups, different OBJs
    BY_MATERIAL = 2  # Seperate geometry based on material


def LoadOBJ(
    path: str,
    group_type: OBJGroupType = OBJGroupType.ONLY_FILE,
    rotation: Tuple[float, float, float] = (0, 0, 0),
) -> MeshInfo:
    """ Load an obj file from the given path. 
    
    Args:
        path: The path to a valid OBJ file.
        group_type: (NOT IMPLEMENTED) How IDs will be assigned to different
            parts of the mesh 
        rotation: Rotation in degrees to be performed on the OBJ after it is loaded. 
            Useful for converting models from Y-Up to Z-Up.
    
    Returns:
        MeshInfo: A new meshinfo object containing a the vertices and triangles
            for the obj in path.
   
    Raises:
        humanfactorspy.Exceptions.InvalidOBJException: The OBJ at path
            either did not exist or could not be loaded
        humanfactorspy.Exceptions.FileNotFoundException: No file exists
            at the given path.

    Example:
        Load plane.obj from a folder titled "Example Models" but fail because that file doesn't exist
        
        >>> import humanfactorspy
        >>> from humanfactorspy.geometry import LoadOBJ
        >>> obj = humanfactorspy.get_sample_model("not_real.obj")
        >>> MI = LoadOBJ(obj)
        Traceback (most recent call last):
        ...
        humanfactorspy.Exceptions.FileNotFoundException

        >>> import humanfactorspy
        >>> from humanfactorspy.geometry import LoadOBJ
        >>> obj = humanfactorspy.get_sample_model("plane.obj")
        >>> MI = LoadOBJ(obj)
        >>> MI 
        <humanfactorspy.geometry.mesh_info.MeshInfo object at ...>


    """

    mesh_ptr = meshinfo_native_functions.CreateOBJ(path, OBJGroupType, rotation)
    return MeshInfo(mesh_ptr)
