from ctypes import *
from dhart.Exceptions import *
from typing import *

from dhart.common_native_functions import getDLLHandle

HFPython: Union[None, CDLL] = None  # The C++ DLL that contains all functionality we need

HFPython = getDLLHandle()


def C_SphericalViewAnalysisAggregate(
    bvh: c_void_p,
    nodes: c_void_p,
    node_count: int,
    ray_count: int,
    height: float,
    agg_type: int,
    upper_fov=50,
    lower_fov=70) -> Tuple[int, c_void_p, c_void_p]:
    """ Conduct view analysis in C++

    Returns:
        int: Size of the vector created
        c_void_p: Pointer to the score vector itself
        c_void_p: Pointer to the data of the pointer

    """

    score_data_ptr = pointer(c_void_p(0))
    score_vector_ptr = pointer(c_void_p(0))
    vector_size = c_int(0)

    HFPython.SphereicalViewAnalysisAggregate(
        bvh,
        nodes,
        node_count,
        ray_count,
        c_float(upper_fov),
        c_float(lower_fov),
        c_float(height),
        agg_type,
        byref(score_vector_ptr),
        byref(score_data_ptr),
        byref(vector_size),
    )

    return (vector_size.value, score_vector_ptr, score_data_ptr)


def C_SphericalViewAnalysis(bvh: c_void_p, nodes: c_void_p, node_count: int,
                            ray_count: int, height: float, upper_fov=50, lower_fov=70
                            ) -> Tuple[c_void_p, c_void_p]:
    """ Perform view analysis and get the result of each raycast individually

    This function casts node_count * ray_count rays

    Returns:
        c_void_p: Points to a ViewAnalysisResult vector.
        c_void_p: Points to the vector's underlying data. The vector is laid out contiguously with shape (node_count, ray_count). Within each element in this array is a float for distance, and an int for meshid
    Raises:
        HF.Exceptions.OutOfMemoryException: The number of rays/nodes was too large to fit in a C++ vector

    """

    result_data_ptr = pointer(c_void_p(0))
    result_vector_ptr = pointer(c_void_p(0))
    num_rays = c_int(ray_count)
    HFPython.SphericalViewAnalysisNoAggregate(
        bvh,
        nodes,
        node_count,
        byref(num_rays),
        c_float(upper_fov),
        c_float(lower_fov),
        c_float(height),
        byref(result_vector_ptr),
        byref(result_data_ptr),
    )
    # print("Num Rays=", ray_count, num_rays.value)

    return (result_vector_ptr, result_data_ptr, num_rays.value)


def C_DistributeSpherical(
    num_rays: int, upper_fov=50, lower_fov=70
) -> Tuple[c_void_p, c_void_p]:
    vector_ptr = c_void_p(0)
    data_ptr = c_void_p(0)
    ray_count = c_int(num_rays)

    HFPython.SphericalDistribute(
        byref(ray_count),
        byref(vector_ptr),
        byref(data_ptr),
        c_float(upper_fov),
        c_float(lower_fov),
    )

    return vector_ptr, data_ptr, ray_count.value


def C_DestroyScores(vector_ptr):
    HFPython.DestroyFloatVector(vector_ptr)
