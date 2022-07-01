import ctypes
import numpy
from humanfactorspy.geometry import meshinfo_native_functions
import itertools
from typing import *

""" Mesh Info.

Contains the MeshInfo Type.
"""

__all__ = ['Flatten','MeshInfo','ConstructPlane']

def Flatten(list_of_objects: List[Tuple[Any]]) -> List[Any]:
    """ Flatten a list of list of tuples """
    return list(itertools.chain.from_iterable(list_of_objects))


class MeshInfo(object):
    """ A mesh stored in C++ as a 3*n array. 

    MeshInfo is required in order to create Embree BVhs
    
    """
    def __init__(
        self,
        indices_or_pointer: Union[
            List[Tuple[int, int, int]], List[int], ctypes.c_void_p, str
        ],
        vertices: Union[List[Tuple[float, float, float]], List[float], None] = None,
        name: Union[None, str] = "",
        id: Union[None, int] = 39,
    ):
        """ Create a new meshinfo from either a pointer to an existing mesh object or vertices for a mesh.
        
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


        Examples:
            Manually construct a flat plane, by giving the vertices and triangles straight
            to the mesh info constructor. 
            
            >>> from humanfactorspy.geometry import MeshInfo
            >>> vertices = [(-10, 0.0, 10), (-10, 0.0, -10),(10, 0.0, 10),(10, 0.0, 10),]
            >>> tris = [[3, 1, 0], [2, 3, 0]]
            >>> MI = MeshInfo(tris, vertices)

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

        # Setup vertex and triangle arrays
        self.SetupVertAndIndexArrays()

    def SetupVertAndIndexArrays(self):
        """ Update name, id, and vert/index arrays from C++
        """
        # Get name and ID from C++
        self.name = meshinfo_native_functions.C_GetMeshName(self.__internal_ptr)
        self.id = meshinfo_native_functions.C_GetMeshID(self.__internal_ptr)

        # Get ptrs to and the size of vertex and triangle arrays
        (
            triangle_ptr,
            num_tris,
            vertex_ptr,
            num_verts,
        ) = meshinfo_native_functions.C_GetMeshVertsAndTris(self.__internal_ptr)

        # Map to numpy arrays
        tri_arr_ptr = ctypes.cast(triangle_ptr, ctypes.POINTER(ctypes.c_int))
        self.indices = numpy.ctypeslib.as_array(tri_arr_ptr, shape=(num_tris, 3))

        vert_arr_ptr = ctypes.cast(vertex_ptr, ctypes.POINTER(ctypes.c_float))
        self.vertices = numpy.ctypeslib.as_array(vert_arr_ptr, shape=(num_verts, 3))

    def Rotate(self, rotation: Tuple[float, float, float]) -> None:
        """ Rotate this mesh by the given rotation

        Args:
            rotation: number of degrees to rotate the mesh on the X Y and Z axis. 
        """
        if not isinstance(rotation, (tuple, list)):
            raise TypeError("rotation was not of the correct type")

        meshinfo_native_functions.C_RotateMesh(self.__internal_ptr, rotation)

    def __del__(self):
        """ Clean up objects in C++ upon garbage collection """
        if self.__internal_ptr is not None:
            meshinfo_native_functions.DestroyMeshInfo(self.__internal_ptr)
    
    def __str__(self):
        return f"({self.name}, {self.id})"

    def __repr__(self):
        return f"({self.name}, {self.id})"

def ConstructPlane():
    """ Construct a new plane object. Used internally for testing """

    vertices = [
        (-10, 0.0, 10),
        (-10, 0.0, -10),
        (10, 0.0, 10),
        (10, 0.0, 10),
    ]
    tris = [[3, 1, 0], [2, 3, 0]]

    return MeshInfo(tris, vertices)
