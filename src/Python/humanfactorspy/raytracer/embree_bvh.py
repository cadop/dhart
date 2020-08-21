from humanfactorspy.raytracer import raytracer_native_functions
from humanfactorspy.geometry import MeshInfo
from typing import *
from ctypes import c_void_p

__all__ = ['EmbreeBVH']

class EmbreeBVH(object):
    """ An acceelerated datastructure optimized for high performane ray intersections
    
    Note:
        A BVH is an accelerated data structure that's used for high performance
        ray intersections. All functions that ustilize the embree ray tracer
        require the mesh first to be converted to this format. 

    """

    pointer: Union[None, c_void_p] = None  # Pointer to the underlying c-object

    def __init__(self, geometry: Union[MeshInfo, List[MeshInfo]]):
        """ Create a new BVH from an existing mesh 

        Args:
            geometry: The mesh or meshes to create the BVH from. 
        
        Example:
            Creating a BVH from a plane object

            >>> from humanfactorspy.geometry import ConstructPlane
            >>> from humanfactorspy.raytracer import EmbreeBVH

            >>> MI = ConstructPlane()
            >>> BVH = EmbreeBVH(MI)

        """
        
        if isinstance(geometry, List):
            pointers = [mesh._MeshInfo__internal_ptr for mesh in geometry]
        else:
            pointers = geometry._MeshInfo__internal_ptr
        self.pointer = raytracer_native_functions.CreateRayTracer(
            pointers
        )

    def AddMesh(self, mesh : Union[MeshInfo, List[MeshInfo]]):
        """Add one or more new meshes to the BVH

        Args:
            mesh (Union[MeshInfo, List[MeshInfo]]): One or more instances of meshinfo to be added to the BVH.

        Note:
            The IDs of each mesh will be updated to new values
            in the case that they collide with the id of any 
            exiting meshes already in the BVH. 
        """
        # If it's a list, construct a list of pointers, otherwise
        # create a list containing only a pointer to the single mesh
        if isinstance(mesh, list):
            pointers = [m._MeshInfo__internal_ptr for m in mesh] 
        else:
            pointers = [mesh._MeshInfo__internal_ptr]

        # Add the meshes in c++
        raytracer_native_functions.C_AddMeshes(self.pointer, pointers)

        # Update params from C++ in the case that the ID of any mesh changed.
        if isinstance(mesh, list):
            for m in mesh:
                m.SetupVertAndIndexArrays()
        else:
            mesh.SetupVertAndIndexArrays()


    def __del__(self):
        if self.pointer != 0:
            raytracer_native_functions.DestroyRayTracer(self.pointer)

