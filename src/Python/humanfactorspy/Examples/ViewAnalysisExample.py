
from humanfactorspy.geometry import LoadOBJ, CommonRotations
from humanfactorspy.raytracer import EmbreeBVH
from humanfactorspy.viewanalysis import SphericalViewAnalysis, SphericalViewAnalysisAggregate, AggregationType

# Load BVH
obj_path = "H:\\HumanMetrics\\Codebase\\HumanFactors\\out\\install\\x64-Debug\\Example Models\\plane.obj"

loaded_obj = LoadOBJ(obj_path, rotation=CommonRotations.Yup_to_Zup)
bvh = EmbreeBVH(loaded_obj)

p1 = (0,0,2)
dir = (0,0,-1)
ray_count = 1000
height = 1.7

results = SphericalViewAnalysis(bvh, p1, ray_count, height)
aggregate_results = SphericalViewAnalysisAggregate(bvh, p1, ray_count, height, AggregationType.AVERAGE)

print(results)
print(aggregate_results)