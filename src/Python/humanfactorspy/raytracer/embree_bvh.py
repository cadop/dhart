from humanfactorspy.raytracer import raytracer_native_functions
from humanfactorspy.geometry import MeshInfo
from typing import *
from ctypes import c_void_p


class EmbreeBVH(object):
    """Holds a BVH for the embree raytracer.
    
    Import embree_raytracer to use this
    """

    pointer: Union[None, c_void_p] = None  # Pointer to the underlying c-object

    def __init__(self, geometry: MeshInfo):
        """ Create a new BVH from meshinfo """
        self.pointer = raytracer_native_functions.CreateRayTracer(
            geometry._MeshInfo__internal_ptr
        )

    def __del__(self):
        if self.pointer != 0:
            raytracer_native_functions.DestroyRayTracer(self.pointer)

