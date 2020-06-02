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

    Args:
        bvh: the BVH for the geometry you're shooting at
        nodes: A list of tuples containing x,y,z coordinates of points to analyze
        ray_count: Amount of rays to shoot
        height: height to offset nodes from the ground in meters
        upward_fov: maximum angle up from the user's eyelevel  to be considred
        downward_fov: maximum angle below from the user's eyelevel to be considred
        AT: aggregation method to use for distance. 

    Examples:
        Fire 150 rays for 3 nodesat 1.7m, then get the sum of the distance to every hit for all three

        >>> from humanfactorspy.geometry import CommonRotations
        >>> from humanfactorspy.raytracer import EmbreeBVH  
        >>> from humanfactorspy.geometry.mesh_info import ConstructPlane
        >>> from humanfactorspy.viewanalysis import SphericalViewAnalysisAggregate, AggregationType

        >>> MI = ConstructPlane()
        >>> MI.Rotate(CommonRotations.Zup_to_Yup)
        >>> BVH = EmbreeBVH(MI)
        >>> origins = [(0,0,1), (1,0,1), (2,0,50)]
        >>> va = SphericalViewAnalysisAggregate(BVH, origins, 100, 1.7, agg_type=AggregationType.SUM)
        >>> print(va)
        [95.05698 73.62985  0.     ]

        Fire 150 rays for 3 nodesat 1.7m, then get the average distance for all three
        
        >>> from humanfactorspy.geometry import CommonRotations
        >>> from humanfactorspy.raytracer import EmbreeBVH  
        >>> from humanfactorspy.geometry.mesh_info import ConstructPlane
        >>> from humanfactorspy.viewanalysis import SphericalViewAnalysisAggregate, AggregationType

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

        Example:
        Conducting view analysis on 2 nodes firing 10 rays.
        
        >>> from humanfactorspy.geometry import LoadOBJ, MeshInfo, CommonRotations
        >>> from humanfactorspy.raytracer import EmbreeBVH  
        >>> from humanfactorspy.geometry.mesh_info import ConstructPlane
        >>> from humanfactorspy.viewanalysis import SphericalViewAnalysis

        >>> MI = ConstructPlane()
        >>> MI.Rotate(CommonRotations.Zup_to_Yup)
        >>> BVH = EmbreeBVH(MI)
        >>> origins = [(0,0,1), (1,0,1)]
        >>> va = SphericalViewAnalysis(BVH, origins, 10, 1.7)
        >>> print(va)
        [[(-1.      , -1) (-1.      , -1) (-1.      , -1) (-1.      , -1)
          (-1.      , -1) ( 5.40725 , 39) (-1.      , -1) (-1.      , -1)
          ( 3.529445, 39) (-1.      , -1)]
         [(-1.      , -1) (-1.      , -1) (-1.      , -1) (-1.      , -1)
          (-1.      , -1) ( 5.40725 , 39) (-1.      , -1) (-1.      , -1)
          ( 3.529445, 39) (-1.      , -1)]]
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

    Example:
        >>> from humanfactorspy.viewanalysis import SphericallyDistributeRays
        >>> print(SphericallyDistributeRays(10))
        [[-0.         -1.          0.        ]
         [-0.26500335 -0.8181818  -0.51024675]
         [ 0.7245825  -0.63636374  0.2646158 ]
         [-0.8233362  -0.45454547  0.33986175]
         [ 0.40777823 -0.2727272  -0.87139934]
         [-0.86162955  0.09090906 -0.49932963]
         [ 0.93965095  0.2727273  -0.20658147]
         [-0.51228005  0.45454556  0.72866833]
         [-0.09913298  0.6363636  -0.7649929 ]
         [ 0.4396428   0.8181818   0.37053034]]
    Returns:
        A two dimensional array of directions
    
    """

    vector_ptr, data_ptr, num_rays = viewanalysis_native_functions.C_DistributeSpherical(num_rays, upward_fov, downward_fov)
    return ViewAnalysisDirections(vector_ptr, data_ptr, num_rays)
