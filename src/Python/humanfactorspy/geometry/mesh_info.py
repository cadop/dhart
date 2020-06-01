import ctypes
from humanfactorspy.geometry import meshinfo_native_functions
import itertools
from typing import *


def Flatten(list_of_objects: List[Tuple[Any]]) -> List[Any]:
    """ Flatten a list of list of tuples """
    return list(itertools.chain.from_iterable(list_of_objects))


class MeshInfo(object):
    """Information about a mesh from C++"""

    def __init__(
        self,
        indices_or_pointer: Union[
            List[Tuple[int, int, int]], List[int], ctypes.c_void_p, str
        ],
        vertices: Union[List[Tuple[float, float, float]], List[float], None] = None,
        name: Union[None, str] = None,
        id: Union[None, int] = None,
    ):
        """ Create a new meshinfo from either a pointer to an existing mesh object
                or vertices for a mesh.
        
        Args:
            indices_or_pointer: Either a pointer to a valid mesh object from LoadOBJ, 
                or indicies as a list of 3 element tuples or a flat list of integers representing
                the faces of the mesh. Also supports passing a filepath to an OBJ
            vertices : a flat list of floats or a list of 3 index tuples representing the vertices of the mesh.
                If this is specified then the first argument MUST be a list of indices
            name: The name of the mesh. Only supported if not supplying a pointer
            id: Assign a specific ID to this mesh. Only supported if not supplying a pointer

        Raises:
            InvalidOBJException: The list of indices/vertices don't create a valid mesh
        """

        if isinstance(indices_or_pointer, ctypes.c_void_p):
            self.__internal_ptr = indices_or_pointer
        elif isinstance(indices_or_pointer, str):
            self.__internal_ptr = meshinfo_native_functions.CreateOBJ(
                indices_or_pointer
            )
        else:
            # Flatten list of tuples/floats if necessary
            if isinstance(indices_or_pointer[0], (tuple, list)):
                indices_or_pointer = Flatten(indices_or_pointer)
            if isinstance(vertices[0], (tuple, list)):
                vertices = Flatten(vertices)

            # Call out to the CreateMesh function
            self.__internal_ptr = meshinfo_native_functions.CreateMesh(
                indices_or_pointer, vertices, name, id
            )

    def Rotate(self, rotation: Tuple[float, float, float]) -> None:
        """ Rotate the current mesh """
        if not isinstance(rotation, (tuple, list)):
            raise TypeError("rotation was not of the correct type")
        meshinfo_native_functions.C_RotateMesh(self.__internal_ptr, rotation)

    def __del__(self):
        """ Clean up objects in C++ upon garbage collection """
        if self.__internal_ptr is not None:
            meshinfo_native_functions.DestroyMeshInfo(self.__internal_ptr)
