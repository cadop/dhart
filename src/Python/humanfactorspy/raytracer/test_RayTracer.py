import pytest

from humanfactorspy.geometry import LoadOBJ, CommonRotations
from humanfactorspy.raytracer import EmbreeBVH
from humanfactorspy.raytracer.embree_raytracer import *

from time import time

import humanfactorspy
# Setup


def test_BVHCreation():
    mesh_path = humanfactorspy.get_sample_model("sponza.obj") 
    obj = LoadOBJ(mesh_path, rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(obj)


def test_FireRay():
    plane = LoadOBJ(humanfactorspy.get_sample_model("plane.obj"), rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(plane)
    origins = [(0, 0, 1), (1, 1, 1), (-1, -1, 1)]
    direction = (0, 0, -1)

    hit_points = []
    for origin in origins:
        hit_points.append(IntersectForPoint(bvh, origin, direction, 100))

    height = None
    for hit_point in hit_points:
        print(hit_point)
        if not hit_point:
            pytest.fail("Ray didn't connect")
        if not height:
            height = hit_point[2]
        else:
            pytest.approx(height, hit_point[2])


def test_FireMultipleOfTheSameRay():
    plane = LoadOBJ(humanfactorspy.get_sample_model("plane.obj"), rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(plane)
    origins = [(0, 0, 1), (0, 0, 1), (0, 0, 1)]
    directions = [(0, 0, -1), (0, 0, -1), (0, 0, -1)]

    hit_points = IntersectForPoint(bvh, origins, directions, -1)

    if not (hit_points[0] == hit_points[1] and hit_points[1] == hit_points[2]):
        print(hit_points)
        pytest.fail("Some points had different results")


def test_MultipleOriginSameDirection():
    plane = LoadOBJ(humanfactorspy.get_sample_model("plane.obj"), rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(plane)
    origins = [(1, 0, 1), (1, 1, 1), (0, 0, 1)]
    directions = (0, 0, -1)

    hit_points = IntersectForPoint(bvh, origins, directions, -1)

    if not (
        round(hit_points[0][2], 3) == round(hit_points[1][2], 3)
        and round(hit_points[1][2], 3) == round(hit_points[2][2], 3)
    ):
        print(hit_points)
        pytest.fail("Some points had different results")


def test_MultipleDirectionSameOrigin():
    plane = LoadOBJ(humanfactorspy.get_sample_model("plane.obj"), rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(plane)
    origins = [(0, 0, 1)]
    directions = [(0, 0, -1), (0, 0, 1), (0, 1, 0)]

    hit_points = IntersectForPoint(bvh, origins, directions, -1)

    if hit_points[1] or hit_points[2]:
        print("Points hit that should not have")
        print(hit_points)
        pytest.fail()
    elif not hit_points[0]:
        print("Points didn't hit that should have")
        print(hit_points)
        pytest.fail()


def test_MultipleRaysMiss():
    plane = LoadOBJ(humanfactorspy.get_sample_model("plane.obj"), rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(plane)
    origins = [(0, 0, 1), (0, 0, 1), (0, 0, 1)]
    directions = [(0, 0, 1), (0, 1, 0), (1, 0, 0)]

    hit_points = IntersectForPoint(bvh, origins, directions, -1)

    for hit_point in hit_points:
        if hit_point:
            print(hit_points)
            pytest.fail()


def test_MultipleOcclusionRays():
    plane = LoadOBJ(humanfactorspy.get_sample_model("plane.obj"), rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(plane)
    origins = [(0.5, 0, 1), (0, 0.5, 1), (0, 0, 1)]
    directions_that_should_miss = [(0, 0, 1), (0, 1, 0), (1, 0, 0)]
    directions_that_should_hit = [(0, 0, -1), (0, 0, -1), (0, 0, -1)]

    should_hit = IntersectOccluded(bvh, origins, directions_that_should_hit, -1)
    should_miss = IntersectOccluded(bvh, origins, directions_that_should_miss, -1)

    if True in should_miss or False in should_hit:
        print("Rays that should/shouldnt hit hit/didn't hit")
        print("Should Miss: " + str(should_miss))
        print("Should Hit: " + str(should_hit))
        pytest.fail("Rays that should/shouldnt hit hit/didn't hit")
    pass


def test_SingleOcclusionRays():
    plane = LoadOBJ(humanfactorspy.get_sample_model("plane.obj"), rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(plane)
    origin = (0, 0, 1)
    directions_that_should_miss = [(0, 0, 1), (0, 1, 0), (1, 0, 0)]
    directions_that_should_hit = [(0, 0, -1), (0, 0, -1), (0, 0, -1)]

    should_hit = [
        IntersectOccluded(bvh, origin, direction, -1)
        for direction in directions_that_should_hit
    ]
    should_miss = [
        IntersectOccluded(bvh, origin, direction, -1)
        for direction in directions_that_should_miss
    ]

    if True in should_miss or False in should_hit:
        print("Rays that should/shouldnt hit hit/didn't hit")
        print("Should Miss: " + str(should_miss))
        print("Should Hit: " + str(should_hit))
        pytest.fail("Rays that should/shouldnt hit hit/didn't hit")
    pass


def test_FireRayDistance():
    import numpy
    plane = LoadOBJ(humanfactorspy.get_sample_model("plane.obj"), rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(plane)
    origin = (0, 0, 1)
    direction = (0, 0, -1)
    result = Intersect(bvh, origin, direction, -1.0,)
    print(result)
    assert result[0] != -1.0 and not numpy.isnan(result[0])


def test_MultiRayDistancereturnType():
    """ Tests if the return type of Intersect is as the
        documentation states
    """
    # Setup raytracer
    plane = LoadOBJ(
        humanfactorspy.get_sample_model("plane.obj"),
        rotation=CommonRotations.Yup_to_Zup,
    )
    bvh = EmbreeBVH(plane)

    # Ensure these return types are tuples as the docs state
    assert isinstance(Intersect(bvh, [0, 0, 1], [0, 0, -1]), tuple)
    assert isinstance(Intersect(bvh, (0, 0, 1), (0, 0, -1)), tuple)
    assert isinstance(Intersect(bvh, (0, 0, 1), [0, 0, -1]), tuple)

    # Assert that these are lists as the docs state
    assert isinstance(Intersect(bvh, [0, 0, 1], [(0, 0, -1)]), RayResultList)
    assert isinstance(Intersect(bvh, [[0, 0, 1]], (0, 0, -1)), RayResultList)
    assert isinstance(Intersect(bvh, [[0, 0, 1], [0, 0, 2]], (0, 0, -1)), RayResultList)

def test_FireMultipleRayDistance():
    plane = LoadOBJ(humanfactorspy.get_sample_model("plane.obj"), rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(plane)
    origins = [(0, 0, 1), (0, 0, 2), (0, 0, 3)] * 5000

    direction = (0, 0, -1)

    start = time()
    result = Intersect(bvh, origins, direction, -1.0,)
    end = time()
    print(end - start, "s for", len(origins), "rays")

    result_array = result.array

    print(result_array)
    print(result_array.shape)

    for res in result_array:
        mesh_id = res[1]
        distance = res[0]
        assert mesh_id >= 0 and distance >= 1


def test_DoublePrecisionRayCast():
    """ Tests if the return type of Intersect is as the
        documentation states
    """
    # Setup raytracer
    plane = LoadOBJ(
        humanfactorspy.get_sample_model("plane.obj"),
        rotation=CommonRotations.Yup_to_Zup,
    )

    bvh = EmbreeBVH(plane)
    int_should_hit = IntersectDistanceDouble(bvh, (0,0,100), (0,0,-1))
    int_should_miss = IntersectDistanceDouble(bvh, (0,0,100), (0,22,0))

    assert(abs(int_should_hit - 100) < 0.00001 )
    assert(int_should_miss == -1.0)
