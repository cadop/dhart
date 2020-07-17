
from humanfactorspy.geometry import LoadOBJ, CommonRotations
from humanfactorspy.raytracer import (EmbreeBVH,Intersect,
                                        IntersectForPoint,
                                        IntersectOccluded)
import humanfactorspy

# Load BVH
# obj_path = "H:\\HumanMetrics\\Codebase\\HumanFactors\\out\\install\\x64-Debug\\Example Models\\plane.obj"

obj_path = humanfactorspy.get_sample_model('plane.obj')

loaded_obj = LoadOBJ(obj_path, rotation=CommonRotations.Yup_to_Zup)
bvh = EmbreeBVH(loaded_obj)

p1 = (0, 0, 2)
dir = (0, 0, -1)

# Fire a ray for the hitpoint
hit_point = IntersectForPoint(bvh, p1, dir, -1)
print(f"Hit point: {hit_point}")

# Fire a ray for distance/meshid
distance, mesh_id = Intersect(bvh, p1, dir, -1)
print(f"distance is {distance}, meshid is {mesh_id}")

# See if it occludes
does_occlude = IntersectOccluded(bvh, p1, (0, 0, -1), 9999)
print(f"Does the ray connect? {does_occlude}")
