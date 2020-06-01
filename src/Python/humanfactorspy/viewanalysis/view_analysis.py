from ctypes import c_void_p
from typing import *
from enum import Enum

from humanfactorspy.spatialstructures import NodeList
from humanfactorspy.spatialstructures.node import CreateListOfNodeStructs
from humanfactorspy.raytracer import EmbreeBVH, RayResultList

from . import viewanalysis_native_functions
from .view_analysis_scores import ViewAnalysisAggregates, ViewAnalysisDirections


class AggregationType(Enum):
    """ Aggregation method to use for view analysis """

    COUNT = 0  # Number of hits
    SUM = 1  # Sum of the distance from the origin to every hit
    AVERAGE = 2  # The average distance for every node
    MAX = 3  # the longest distance hit
    MIN = 4  # The shortest distance hit


def SphericalViewAnalysisAggregate(
    bvh: EmbreeBVH,
    nodes: Union[List[Tuple[float, float, float]], NodeList],
    ray_count: int,
    height: float,
    upward_fov=50,
    downward_fov=70,
    agg_type: AggregationType = AggregationType.SUM,
) -> ViewAnalysisAggregates:
    """ Conduct view analysis on every node in nodes and aggregate the results

    ray_count rays are evenly distributed around each node in nodes and fired at the
    geometry in bvh. Hits are collected then summarized using the provided aggregation
    method. The actual number of rays fired may be slightly more or less than the amount specified,
    based on the fov limitations specified. 

    Example::
        # Fire 150 rays for nodes 1 and 2 at 1.7m, then get the average distance for both
        Node1 = (0,0,1)
        Node2 = (0,0,2)
        results = SphereicalViewAnalysis(bvh,[Node1, Node2], 150, 1.7, AggregationType.AVERAGE)
        Node1_average_distance = results.array[0]
        Node2_average_distance = results.array[1]

    Args:
        bvh: the BVH for the geometry you're shooting at
        nodes: A list of tuples containing x,y,z coordinates of points to analyze
        ray_count: Amount of rays to shoot
        height: height to offset nodes from the ground in meters
        upward_fov: maximum angle up from the user's eyelevel  to be considred
        downward_fov: maximum angle below from the user's eyelevel to be considred
        AT: aggregation method to use for distance. 

    Returns:
        ViewAnalysisAggregates: view analysis scores resulting from the view analysis calculation
    """

    if isinstance(nodes, tuple):
        nodes = [nodes]

    if isinstance(nodes, list):
        np_arr_pts = CreateListOfNodeStructs(nodes)
        data_ptr = np_arr_pts.ctypes.data_as(c_void_p)
        size = len(np_arr_pts)
    else:
        data_ptr = nodes.data_pointer
        size = len(nodes)

    (
        size,
        score_vector_ptr,
        score_data_ptr,
    ) = viewanalysis_native_functions.C_SphericalViewAnalysisAggregate(
        bvh.pointer,
        data_ptr,
        size,
        ray_count,
        height,
        agg_type.value,
        upper_fov=upward_fov,
        lower_fov=downward_fov,
    )

    return ViewAnalysisAggregates(score_vector_ptr, score_data_ptr, size)


def SphericalViewAnalysis(
    bvh: EmbreeBVH,
    nodes: Union[List[Tuple[float, float, float]], NodeList],
    ray_count: int,
    height: float,
    upward_fov=50,
    downward_fov=70,
) -> RayResultList:
    """ Conduct view analysis on every node in nodes and return the result of every ray fired

    The actual number of rays fired may be slightly more or less than the amount specified,
    based on the fov limitations specified. The direction each ray was fired in can be 
    retrieved from the function SphericallyDistributeRays.

    Args:
        bvh: the BVH for the geometry you're shooting at
        nodes: A list of tuples containing x,y,z coordinates of points to analyze
        ray_count: Amount of rays to shoot, evenly distributed in a sphere around the center
        upward_fov: maximum angle up from the user's eyelevel to be considred
        downward_fov: maximum angle down from the user's eyelevel  to be considred
        height: height to offset nodes from the ground in meters

    Returns:
        RayResult: A list of results of shape (ray_count, len(nodes))
    """
    if isinstance(nodes, tuple):
        nodes = [nodes]

    if isinstance(nodes, list):
        np_arr_pts = CreateListOfNodeStructs(nodes)
        data_ptr = np_arr_pts.ctypes.data_as(c_void_p)
        size = len(np_arr_pts)
    else:
        data_ptr = nodes.data_pointer
        size = len(nodes)

    (
        score_vector_ptr,
        score_data_ptr,
        ray_count
    ) = viewanalysis_native_functions.C_SphericalViewAnalysis(
        bvh.pointer,
        data_ptr,
        size,
        ray_count,
        height,
        upper_fov=upward_fov,
        lower_fov=downward_fov,
    )

    print(ray_count)
    return RayResultList(score_vector_ptr, score_data_ptr, size, ray_count)


def SphericallyDistributeRays(num_rays: int, upward_fov : float = 50, downward_fov: float = 70) -> ViewAnalysisDirections:
    """ Distribute rays evenly in a sphere """

    vector_ptr, data_ptr, num_rays = viewanalysis_native_functions.C_DistributeSpherical(num_rays, upward_fov, downward_fov)
    return ViewAnalysisDirections(vector_ptr, data_ptr, num_rays)
