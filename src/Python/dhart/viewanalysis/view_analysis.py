from ctypes import c_void_p
from typing import *
from enum import Enum

import numpy as np

from dhart.spatialstructures import NodeList
from dhart.spatialstructures.node import CreateListOfNodeStructs
from dhart.raytracer import EmbreeBVH, RayResultList

from . import viewanalysis_native_functions
from .view_analysis_scores import ViewAnalysisAggregates, ViewAnalysisDirections

from .. import utils

__all__ = ['AggregationType','SphericalViewAnalysisAggregate','SphericalViewAnalysis','SphericallyDistributeRays']

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

    ray_count rays are evenly distributed around each node in nodes and cast at the
    geometry in bvh. Hits are collected then summarized using the provided aggregation
    method. The actual number of rays cast may be slightly more or less than the amount specified,
    based on the fov limitations specified. 

    Args:
        bvh: the BVH for the geometry you're shooting at
        nodes: A list of tuples containing x,y,z coordinates of points to analyze
        ray_count: Amount of rays to shoot
        height: height to offset nodes from the ground in meters
        upward_fov: maximum angle up from the user's eyelevel  to be considred
        downward_fov: maximum angle below from the user's eyelevel to be considred
        AT: aggregation method to use for distance. 

    Examples:

        Cast 150 rays for 3 nodesat 1.7m, then get the sum of the distance to every hit for all three

        >>> from dhart.geometry import CommonRotations
        >>> from dhart.raytracer import EmbreeBVH  
        >>> from dhart.geometry.mesh_info import ConstructPlane
        >>> from dhart.viewanalysis import SphericalViewAnalysisAggregate, AggregationType

        >>> MI = ConstructPlane()
        >>> MI.Rotate(CommonRotations.Zup_to_Yup)
        >>> BVH = EmbreeBVH(MI)
        >>> origins = [(0,0,1), (1,0,1), (2,0,50)]
        >>> va = SphericalViewAnalysisAggregate(BVH, origins, 100, 1.7, agg_type=AggregationType.SUM)
        >>> print(va)
        [95.05698 73.62985  0.     ]

        Cast 150 rays for 3 nodesat 1.7m, then get the average distance for all three
        
        >>> from dhart.geometry import CommonRotations
        >>> from dhart.raytracer import EmbreeBVH  
        >>> from dhart.geometry.mesh_info import ConstructPlane
        >>> from dhart.viewanalysis import SphericalViewAnalysisAggregate, AggregationType

        >>> MI = ConstructPlane()
        >>> MI.Rotate(CommonRotations.Zup_to_Yup)
        >>> BVH = EmbreeBVH(MI)
        >>> origins = [(0,0,1), (1,0,1), (2,0,50)]
        >>> va = SphericalViewAnalysisAggregate(BVH, origins, 100, 1.7, agg_type=AggregationType.AVERAGE)
        >>> print(va)
        [4.5265236 4.3311677 0.       ]


    Returns:

        ViewAnalysisAggregates: view analysis scores resulting from the view analysis calculation

    """

    # If the input was a single point make it a list
    if utils.is_point(nodes):
        nodes = [nodes]

    np_arr_pts = CreateListOfNodeStructs(nodes)
    data_ptr = np_arr_pts.ctypes.data_as(c_void_p)
    size = len(np_arr_pts)

    (size,score_vector_ptr,score_data_ptr) = viewanalysis_native_functions.C_SphericalViewAnalysisAggregate(
                                                bvh.pointer, 
                                                data_ptr, 
                                                size, 
                                                ray_count, 
                                                height,
                                                agg_type.value, 
                                                upper_fov=upward_fov, 
                                                lower_fov=downward_fov)

    return ViewAnalysisAggregates(score_vector_ptr, score_data_ptr, size)


def SphericalViewAnalysis(
    bvh: EmbreeBVH,
    nodes: Union[List[Tuple[float, float, float]], NodeList],
    ray_count: int,
    height: float,
    upward_fov=50,
    downward_fov=70,
) -> RayResultList:
    """ Conduct view analysis on every node in nodes and return the result of every ray cast

    The actual number of rays cast may be slightly more or less than the amount specified,
    based on the fov limitations specified. The direction each ray was cast in can be 
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

    Examples:

        Conducting view analysis on 2 nodes casting 10 rays.
        
        >>> from dhart.geometry import LoadOBJ, MeshInfo, CommonRotations
        >>> from dhart.raytracer import EmbreeBVH  
        >>> from dhart.geometry.mesh_info import ConstructPlane
        >>> from dhart.viewanalysis import SphericalViewAnalysis

        >>> MI = ConstructPlane()
        >>> MI.Rotate(CommonRotations.Zup_to_Yup)
        >>> BVH = EmbreeBVH(MI)
        >>> origins = [(0,0,1), (1,0,1)]
        >>> va = SphericalViewAnalysis(BVH, origins, 10, 1.7)
        >>> print(va)
        [[(-1.      , -1) (-1.      , -1) (-1.      , -1) (-1.      , -1)
          (-1.      , -1) ( 5.40725 ,  0) (-1.      , -1) (-1.      , -1)
          ( 3.529445,  0) (-1.      , -1)]
         [(-1.      , -1) (-1.      , -1) (-1.      , -1) (-1.      , -1)
          (-1.      , -1) ( 5.40725 ,  0) (-1.      , -1) (-1.      , -1)
          ( 3.529445,  0) (-1.      , -1)]]

    """

    # If the input was a single point make it a list
    if utils.is_point(nodes):
        nodes = [nodes]

    np_arr_pts = CreateListOfNodeStructs(nodes)
    data_ptr = np_arr_pts.ctypes.data_as(c_void_p)
    size = len(np_arr_pts)

    (score_vector_ptr,score_data_ptr,ray_count) = viewanalysis_native_functions.C_SphericalViewAnalysis(
                                                    bvh.pointer,
                                                    data_ptr,
                                                    size,
                                                    ray_count,
                                                    height,
                                                    upper_fov=upward_fov,
                                                    lower_fov=downward_fov)

    return RayResultList(score_vector_ptr, score_data_ptr, size, ray_count)


def SphericallyDistributeRays(num_rays: int, upward_fov : float = 50, downward_fov: float = 70) -> ViewAnalysisDirections:
    """ Distribute directions evenly in a sphere. 
    
    This is the same algorithm used in the other view analysis functions, so the
    output of this can be used to get the hit points of results from 
    SphericalViewAnalysis.

    Args:
        num_rays: the number of directions to generate, Note that the actual number may be higher
        upward_fov: the maximum angle upwards to generate directions for in degrees
        downward_fov: the maximum angle downwards to generate directions for in degrees

    Returns:

        A two dimensional array of directions

    Examples:

        >>> from dhart.viewanalysis import SphericallyDistributeRays
        >>> import numpy as np
        >>> directions = SphericallyDistributeRays(10)
        >>> print(np.round(directions, 4))
        [[-0.     -1.      0.   ]
        [-0.265  -0.8182 -0.5102]
        [ 0.7246 -0.6364  0.2646]
        [-0.8233 -0.4545  0.3399]
        [ 0.4078 -0.2727 -0.8714]
        [-0.8616  0.0909 -0.4993]
        [ 0.9397  0.2727 -0.2066]
        [-0.5123  0.4545  0.7287]
        [-0.0991  0.6364 -0.765 ]
        [ 0.4396  0.8182  0.3705]]

    """

    vector_ptr, data_ptr, num_rays = viewanalysis_native_functions.C_DistributeSpherical(num_rays, upward_fov, downward_fov)
    return ViewAnalysisDirections(vector_ptr, data_ptr, num_rays)
