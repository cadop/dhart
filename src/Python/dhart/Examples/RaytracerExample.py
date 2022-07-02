
"""

In this example we cast an individual ray into a plane that was loaded from an OBJ file. 

.. testcode:: 

    from dhart.geometry import LoadOBJ, CommonRotations
    from dhart.raytracer import (EmbreeBVH,Intersect,
                                            IntersectForPoint,
                                            IntersectOccluded)
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

    # Cast a ray for the hitpoint
    hit_point = IntersectForPoint(bvh, p1, dir, -1)
    print(f"Hit point: {hit_point}")

    # Cast a ray for distance/meshid
    distance, mesh_id = Intersect(bvh, p1, dir, -1)
    print(f"distance is {distance}, meshid is {mesh_id}")

    # See if it occludes
    does_occlude = IntersectOccluded(bvh, p1, (0, 0, -1), 9999)
    print(f"Does the ray connect? {does_occlude}")


.. testoutput:: 

    Hit point: (0.0, 0.0, 0.0)
    distance is 2.0, meshid is 0
    Does the ray connect? True

"""

from dhart.geometry import LoadOBJ, CommonRotations
from dhart.raytracer import (EmbreeBVH,Intersect,
                                        IntersectForPoint,
                                        IntersectOccluded)
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

# Cast a ray for the hitpoint
hit_point = IntersectForPoint(bvh, p1, dir, -1)
print(f"Hit point: {hit_point}")

# Cast a ray for distance/meshid
distance, mesh_id = Intersect(bvh, p1, dir, -1)
print(f"distance is {distance}, meshid is {mesh_id}")

# See if it occludes
does_occlude = IntersectOccluded(bvh, p1, (0, 0, -1), 9999)
print(f"Does the ray connect? {does_occlude}")

