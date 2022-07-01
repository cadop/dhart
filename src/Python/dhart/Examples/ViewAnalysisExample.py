"""
This example shows how to cast rays in a sphere from a given location, then aggregate the distance values to a single number. 

.. testcode:: 

    from dhart.geometry import LoadOBJ, CommonRotations
    from dhart.raytracer import EmbreeBVH
    from dhart.viewanalysis import SphericalViewAnalysis, SphericalViewAnalysisAggregate, AggregationType

    import dhart

    # Get model path
    obj_path = dhart.get_sample_model('plane.obj')
    # Load mesh
    loaded_obj = LoadOBJ(obj_path, rotation=CommonRotations.Yup_to_Zup)
    # Create BVH
    bvh = EmbreeBVH(loaded_obj)

    # Define point to start ray
    p1 = (0, 0, 2)
    # Define direction to cast ray
    dir = (0, 0, -1)
    ray_count = 1000
    height = 1.7

    results = SphericalViewAnalysis(bvh, p1, ray_count, height)
    aggregate_results = SphericalViewAnalysisAggregate(bvh, p1, ray_count, height, agg_type=AggregationType.AVERAGE)

    print(results[15:20])
    print(aggregate_results)

.. testoutput:: 

    [(-1.      , -1) (-1.      , -1) (15.833383,  0) (-1.      , -1)
     (-1.      , -1)]
    [7.43102]

"""


from dhart.geometry import LoadOBJ, CommonRotations
from dhart.raytracer import EmbreeBVH
from dhart.viewanalysis import SphericalViewAnalysis, SphericalViewAnalysisAggregate, AggregationType

import dhart

# Get model path
obj_path = dhart.get_sample_model('plane.obj')
# Load mesh
loaded_obj = LoadOBJ(obj_path, rotation=CommonRotations.Yup_to_Zup)
# Create BVH
bvh = EmbreeBVH(loaded_obj)

# Define point to start ray
p1 = (0, 0, 2)
# Define direction to cast ray
dir = (0, 0, -1)
ray_count = 1000
height = 1.7

results = SphericalViewAnalysis(bvh, p1, ray_count, height)
aggregate_results = SphericalViewAnalysisAggregate(bvh, p1, ray_count, height, agg_type=AggregationType.AVERAGE)

print(results[15:20])
print(aggregate_results)