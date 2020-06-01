import ctypes
from typing import *
import numpy

from humanfactorspy.raytracer import EmbreeBVH
import humanfactorspy.raytracer.raytracer_native_functions as raytracer_native_functions
from humanfactorspy.native_numpy_like import NativeNumpyLike


class ResultStruct(ctypes.Structure):
    """ A struct of results containing distance, and meshid 

    If the ray didn't hit anything, both distance and MeshID are usually set
    to zero.    

    Attributes:
        [0] = distance
        [1] = Mesh_ID
    """

    _fields_ = [("distance", ctypes.c_float), ("meshid", ctypes.c_int)]


class RayResultList(NativeNumpyLike):
    """ A list of results from a set of ray queries. Includes distance/meshid """

    native_type = ResultStruct 
    delete_fp = raytracer_native_functions.DestroyRayResultVector

    def __init__(
        self,
        vector_ptr: ctypes.c_void_p,
        data_ptr: ctypes.c_void_p,
        node_count: int,
        ray_count: int,
    ):
        """ Create a new view analysis result from view analysis in C++
        
        Args:
            vector_ptr: a pointer to a vector of floats
            data_ptr: a pointer to the underlying data of the node vector
            node_count : The number of nodes for this result set
            ray_count : the number of rays fired per node in this result set
        """

        # Make this one dimensional if either node_count or ray_count is 1
        if node_count <= 1 or ray_count <= 1:
            shape = max(node_count, ray_count)
        else:
            shape = (node_count, ray_count)

        super().__init__(
            vector_ptr,
            data_ptr,
            shape,
        )


def isValidBVH(bvh: object):
    """ Check if the given object is actually a BVH before sending the pointer to C++

    If this check were not performed, the pointer from any object with the .pointer attribute
    could be sent to C++ with catastrophic results. This check must be performed to ensure that
    every pointer sent to C++ will lead to a valid and fully initialized Raytracer
        
    Raises:
        TypeError: The passed object was not a valid bvh
    """
    if not isinstance(bvh, EmbreeBVH):
        raise TypeError(
            f"The embree_raytracer was passed a {type(bvh)} instead of a {EmbreeBVH}"
        )
    return True


def Intersect(
    bvh: EmbreeBVH,
    origin: Union[Tuple[float, float, float], List[Tuple[float, float, float]]],
    direction: Union[Tuple[float, float, float], List[Tuple[float, float, float]]],
    max_distance: float = -1.0,
) -> Union[numpy.array, Tuple[float, int]]:
    """ Fire one or more rays to get distance to hit and mesh id
    
    In situations where multiple rays are shot, rays will be fired in parallel. 

    Accepts the following configurations:
        1) Single origin, single direction
        2) Multiple origins, multiple directions
        3) Single origin, multiple directions
        4) Multiple origins, single direction

    Example::

        # Single Ray
        result = FireRayForResults(bvh, (x,y,z), (x,y,z))
        distance, meshid = result[0], result[1]

        # Multiple Rays
        results = FireRayForResults(bvh, [(x,y,z), (x,y,z)], (0,0,-1))
        
        node1_distance, node1_meshid = results.array[0][0], results.array[0][1]
        node2_distance, node2_meshid = results.array[1][0], results.array[1][1]

    Args:
        bvh: A valid Embree BVH
        origin: a single tuple of x,y,z coordinates, or a list of x,y,z tuples
        direction: A single direction or list of directions
        max_distance: the maximum distance tto still be considered a hit for the ray
    
    Returns:
        Union[numpy.array, Tuple[float, int]]: If a single ray, then a tuple of float
            for distance and an int for meshid. If multiple rays, return a numpy array of
            RayResult structs. In all cases, a distance/meshid of -1 indicates a miss.
    """
    if isinstance(origin, tuple) and isinstance(direction, tuple):
        return raytracer_native_functions.FireRaySingleDistance(
            bvh.pointer, origin, direction, max_distance
        )

    else:
        # Determine size of result array
        origin_size = 0
        direction_size = 0
        if isinstance(origin, list):
            origin_size = len(origin)
        if isinstance(direction, list):
            direction_size = len(direction)
        result_size = max(origin_size, direction_size)
        assert result_size != 0

        vector_ptr, array_ptr = raytracer_native_functions.FireMultipleRaysDistance(
            bvh.pointer, origin, direction, max_distance
        )

        return RayResultList(vector_ptr, array_ptr, result_size, 1)


def IntersectForPoint(
    bvh: EmbreeBVH,
    origins: Union[Iterable[Tuple[float, float, float]], Tuple[float, float, float]],
    directions: Union[Iterable[Tuple[float, float, float]], Tuple[float, float, float]],
    max_distance: float = -1,
) -> List[Union[Tuple[float, float, float], None]]:
    """ Fire one or more rays based on input origins and directions 
        and get the hit point.
    
    To shoot multiple rays from one origin, or fire rays from multiple origins
    in a single direction, set origins OR directions to a single value. If
    they are both set to a single value then the ray will be fired as a single 
    ray via FireRay.
    
    Args:
        origins: A list of origins or the origin point to shoot from
        directions: A list of directions or the direction to shoot in
        max_distance: Maximum distance that a ray can travel. Any hits beyond this point
            are not counted
    Returns:
        List: an ordered list containing None where rays did not intersect any geometry
            and tuples of 3 floats where the rays did intersect geometry
    Raises:
        TypeError : When the passed BVH is invalid
    """
    isValidBVH(bvh)
    origins_is_list = False
    directions_is_list = False
    origin = None
    direction = None

    # Check if origins is a list
    if isinstance(origins, list):
        # If origin only has a single element then just take the first
        # value and act as if it was a single instance
        if len(origins) == 1:
            origin = origins[0]
        else:
            origins_is_list = True
    else:
        origin = origins

    # Check if Directions is a list
    if isinstance(directions, list):
        # If directions only has a single element then just take the first
        # value and act as if it was a single instance
        if len(directions) == 1:
            direction = directions[0]
        else:
            directions_is_list = True
    else:
        direction = directions

    # They are both lists so we're firing
    if directions_is_list and origins_is_list:
        # this will cause a problem in C if it isn't caught
        if len(directions) != len(origins):
            print("Length of directions and origins do not match!")
            raise RuntimeError()
        return raytracer_native_functions.FireMultipleRays(
            bvh.pointer, origins, directions, max_distance
        )
    elif directions_is_list and not origins_is_list:
        return raytracer_native_functions.FireOneOriginMultipleDirections(
            bvh.pointer, origin, directions, max_distance
        )
    elif not directions_is_list and origins_is_list:
        return raytracer_native_functions.FireMultipleOriginsOneDirection(
            bvh.pointer, origins, direction, max_distance
        )
    elif not directions_is_list and not origins_is_list:
        return raytracer_native_functions.FireRay(
            bvh.pointer, origin, direction, max_distance
        )


def IntersectOccluded(
    bvh: EmbreeBVH,
    origins: Union[Iterable[Tuple[float, float, float]], Tuple[float, float, float]],
    directions: Union[Iterable[Tuple[float, float, float]], Tuple[float, float, float]],
    max_distance: float = -1,
) -> Union[List[bool], bool]:
    """ Fire one or more occlusion rays in C++

    Occlusion rays are faster than standard rays, however can only return whether
    or not they hit anything. 
    
    Returns:
        List[bool] or bool : an ordered list of booleans where true indicates a
            hit, and false indicates a miss. If a single element is passed, only
            bool is returned.
    
    Args:
        origins : A list of origin points, or a single origin point
        directions : A list of directions or a single direction
        max_distance : Maximum distance that a ray can travel. Any hits beyond this point
            are not counted
    
    Raises:
        TypeError: BVH is not a valid EmbreeBVH
    """
    isValidBVH(bvh)

    if len(origins) == 1 or not isinstance(origins, List):
        origins = (origins[0], origins[1], origins[2])

    res = raytracer_native_functions.FireOcclusionRays(
        bvh.pointer, origins, directions, max_distance
    )

    if len(res) == 1:
        return res[0]
    else:
        return res
