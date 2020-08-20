from ctypes import *
from humanfactorspy.Exceptions import *
from humanfactorspy.common_native_functions import (
    getDLLHandle,
    ConvertPointsToArray,
)
from typing import *

HFPython = getDLLHandle()


def CreateRayTracer(mesh_info_ptr: Union[c_void_p, List[c_void_p]]) -> Union[c_void_p, List[c_void_p]]:
    """ Create a raytracer from a pointer to valid meshinfo previously created by CreateOBJ

    Raises:
        HF.Exceptions.MissingDependencyExceptio : If Embree DLL couldn't be found
    Returns:
        c_void_p: A pointer to a fully constructed EmbreeRayTracer
    """
    rt_ptr = c_void_p(0)

    # If this isn't a list, only call the function for a single value
    if (not isinstance(mesh_info_ptr, list)):
        error_code = HFPython.CreateRaytracer(mesh_info_ptr, byref(rt_ptr))

    # If this is a list, call the multi-mesh version
    else:
        
        # Create an array of c_void_p and fill it with values from input array
        num_ptrs = len(mesh_info_ptr)
        meshinfo_ptrs = (c_void_p * len(mesh_info_ptr))()
        for i in range(0, num_ptrs):
            meshinfo_ptrs[i] = mesh_info_ptr[i]

        # Create the raytracer
        error_code = HFPython.CreateRaytracerMultiMesh(
            meshinfo_ptrs, num_ptrs, byref(rt_ptr)
        )

    # Check Error Codes
    if error_code == HF_STATUS.OK:
        pass
    elif error_code == HF_STATUS.MISSING_DEPEND:
        raise MissingDependencyException
    elif error_code == HF_STATUS.GENERIC_ERROR:
        raise HFException

    # Return raytracer
    return rt_ptr

def FireRay(
    rt_ptr: c_void_p,
    origin: Tuple[float, float, float],
    direction: Tuple[float, float, float],
    max_distance: float = -1.0,
) -> Union[Tuple[float, float, float], None]:
    """ Fire a single ray, get a point in return """

    # Convert inputs to c_types
    c_x = c_float(origin[0])
    c_y = c_float(origin[1])
    c_z = c_float(origin[2])
    result = c_bool(False)

    # Fire the ray. if it hits, x,y, and z are overwritten
    HFPython.FireRay(
        rt_ptr,
        byref(c_x),
        byref(c_y),
        byref(c_z),
        c_float(direction[0]),
        c_float(direction[1]),
        c_float(direction[2]),
        c_float(max_distance),
        byref(result),
    )

    # If result is false, then it didn't hit
    if not result.value:
        return None

    return (c_x.value, c_y.value, c_z.value)


def FireMultipleRays(
    rt_ptr: c_void_p,
    origin: List[Tuple[float, float, float]],
    direction: List[Tuple[float, float, float]],
    max_distance: float,
) -> List[Union[Tuple[float, float, float], None]]:
    """ Shoot rays for multiple direction/origin pairs.
        
    Returns:
        List: an ordered list of results containing None for misses and hitpoints for hits
    """

    num_rays = len(origin)

    origin_array = ConvertPointsToArray(origin)
    direction_array = ConvertPointsToArray(direction)

    result_array_type = c_bool * len(origin)
    result_array = result_array_type()

    HFPython.FireMultipleRays(
        rt_ptr,
        byref(origin_array),
        direction_array,
        c_int(num_rays),
        c_float(max_distance),
        byref(result_array),
    )

    return_array = [None] * num_rays
    print(result_array[1])
    for i in range(0, len(origin)):
        if not result_array[i]:
            return_array[i] = None
        else:
            offset = 3 * i
            return_array[i] = (
                origin_array[offset],
                origin_array[offset + 1],
                origin_array[offset + 2],
            )

    return return_array


def FireOneOriginMultipleDirections(
    rt_ptr: c_void_p,
    origin: Tuple[float, float, float],
    direction: List[Tuple[float, float, float]],
    max_distance: float,
) -> List[Union[Tuple[float, float, float], None]]:
    """ Fire multiple rays from a single origin.

    Returns:
        List: an ordered list of hitspoints or None
    """

    num_rays = len(direction)
    direction_array = ConvertPointsToArray(direction)
    origin_array = ConvertPointsToArray(origin)

    result_array_type = c_bool * num_rays
    result_array = result_array_type()

    HFPython.FireMultipleDirectionsOneOrigin(
        rt_ptr,
        origin_array,
        byref(direction_array),
        c_int(num_rays),
        c_float(max_distance),
        byref(result_array),
    )

    return_array = [None] * num_rays
    for i in range(0, num_rays):
        if not result_array[i]:
            return_array[i] = None
        else:
            offset = 3 * i
            return_array[i] = (
                direction_array[offset],
                direction_array[offset + 1],
                direction_array[offset + 2],
            )

    return return_array


def FireMultipleOriginsOneDirection(
    rt_ptr: c_void_p,
    origin: List[Tuple[float, float, float]],
    direction: Tuple[float, float, float],
    max_distance: float,
) -> List[Union[Tuple[float, float, float], None]]:
    """ Fire multiple rays in the same direction """

    num_rays = len(origin)
    direction_array = ConvertPointsToArray(direction)
    origin_array = ConvertPointsToArray(origin)

    result_array_type = c_bool * num_rays
    result_array = result_array_type()

    HFPython.FireMultipleOriginsOneDirection(
        rt_ptr,
        byref(origin_array),
        direction_array,
        c_int(num_rays),
        c_float(max_distance),
        byref(result_array),
    )

    return_array = [None] * num_rays
    for i in range(0, num_rays):
        if not result_array[i]:
            return_array[i] = None
        else:
            offset = 3 * i
            return_array[i] = (
                origin_array[offset],
                origin_array[offset + 1],
                origin_array[offset + 2],
            )

    return return_array


def FireOcclusionRays(
    rt_ptr: c_void_p,
    origin: Union[Tuple[float, float, float], List[Tuple[float, float, float]]],
    direction: Union[Tuple[float, float, float], List[Tuple[float, float, float]]],
    max_distance: float,
) -> List[bool]:
    """ Fire one or more Occlusion Rays """
    if isinstance(origin, tuple):
        num_origins = 1
    else:
        num_origins = len(origin)

    if isinstance(direction, tuple):
        num_directions = 1
    else:
        num_directions = len(direction)

    origin_array = ConvertPointsToArray(origin)
    direction_array = ConvertPointsToArray(direction)

    result_size = max(num_directions, num_origins)
    result_array_type = c_bool * result_size
    result_array = result_array_type()

    HFPython.FireOcclusionRays(
        rt_ptr,
        origin_array,
        direction_array,
        c_int(num_origins),
        c_int(num_directions),
        c_float(max_distance),
        result_array,
    )

    return_array = [False] * result_size
    for i in range(0, result_size):
        return_array[i] = result_array[i]

    return return_array


def FireRaySingleDistance(
    bvh_ptr: c_void_p, origin, direction, max_distance
) -> Tuple[float, int]:
    out_distance = c_float(-1)
    out_meshid = c_int(-1)

    origin_array = ConvertPointsToArray(origin)
    direction_array = ConvertPointsToArray(direction)
    HFPython.FireSingleRayDistance(
        bvh_ptr,
        origin_array,
        direction_array,
        c_float(max_distance),
        byref(out_distance),
        byref(out_meshid),
    )

    return (out_distance.value, out_meshid.value)


def FireMultipleRaysDistance(
    bvh_ptr: c_void_p,
    origins: Union[Tuple[float, float, float], List[Tuple[float, float, float]]],
    directions: Union[Tuple[float, float, float], List[Tuple[float, float, float]]],
    max_distance: float = -1,
) -> Tuple[c_void_p, c_void_p]:
    """
    Returns:
        c_void_p: Pointer to the vector containing the ray results
        c_void_p: Pointer to the vector's underlying data
    """
    origin_array = ConvertPointsToArray(origins)
    direction_array = ConvertPointsToArray(directions)

    num_origins = int(len(origin_array) / 3)
    num_directions = int(len(direction_array) / 3)

    vector_ptr = pointer(c_void_p(0))
    array_ptr = pointer(c_void_p(0))

    res = HFPython.FireRaysDistance(
        bvh_ptr,
        origin_array,
        c_int(num_origins),
        direction_array,
        c_int(num_directions),
        byref(vector_ptr),
        byref(array_ptr),
    )
    if res != HF_STATUS.OK:
        raise Exception("Invalid input")

    return (vector_ptr, array_ptr)




def DestroyRayTracer(rt_ptr: c_void_p):
    """ Call the destructor for a raytracer """
    HFPython.DestroyRayTracer(rt_ptr)


def DestroyRayResultVector(vector_ptr):
    HFPython.DestroyRayResultVector(vector_ptr)
