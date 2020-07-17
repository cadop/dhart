import pytest

from humanfactorspy.geometry import LoadOBJ, CommonRotations
from humanfactorspy.raytracer import EmbreeBVH
from humanfactorspy.viewanalysis.view_analysis import (SphericalViewAnalysisAggregate,
                                                        AggregationType,SphericalViewAnalysis,
                                                        SphericallyDistributeRays)

import humanfactorspy

from time import time

# Setup

def test_Aggregates():
    mesh_path = humanfactorspy.get_sample_model("plane.obj")
    obj = LoadOBJ(mesh_path,rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(obj)


    points = [(0, 0, 1), (0, 0, 2), (0, 0, 3), (0, 0, 4)]
    scores = SphericalViewAnalysisAggregate(
        bvh, points, 100, 1.7, agg_type=AggregationType.COUNT
    )

    assert len(scores.array) == len(points)

def test_AggregateCorrectness():
    mesh_path = humanfactorspy.get_sample_model("plane.obj")
    obj = LoadOBJ(mesh_path, rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(obj)

    points = [(0, 0, 1), (0, 0, 2), (0, 0, 3), (0, 0, 4), (0,0,5), (0,0,6)]
    scores = SphericalViewAnalysisAggregate(
        bvh, points, 10000, 1.7,90,90,agg_type=AggregationType.COUNT,
    )
    
    print("Points:" , points)
    print("Scores:", scores.array)
    for i in range(1, len(points)):
        assert scores.array[i - 1] > scores.array[i]


def test_NonAggregate():
    mesh_path = humanfactorspy.get_sample_model("plane.obj")
    obj = LoadOBJ(mesh_path, rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(obj)

    points = [(1, 1, 1), (2, 2, 2), (3, 3, 3), (4, 4, 4)]*4
    
    num_rays = 10000
    num_nodes = len(points)

    start = time()
    scores = SphericalViewAnalysis(
        bvh, points, num_rays, 1.7, 90, 90
    )
    end = time()
    print("Time =",end-start,"s","for", num_rays * len(points), "rays")
    assert(scores.array.shape == (num_nodes, num_rays))
    print(scores.array)


def test_SphereDistribute():
    num_rays = 10000
    rays = SphericallyDistributeRays(num_rays)
    print(rays)
    assert(rays is not None)
    