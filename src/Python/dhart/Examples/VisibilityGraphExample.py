"""

.. testcode::

    from dhart.geometry import LoadOBJ, CommonRotations
    from dhart.raytracer import EmbreeBVH
    from dhart.visibilitygraph import VisibilityGraphAllToAll

    import dhart

    obj_path = dhart.get_sample_model("plane.obj")
    loaded_obj = LoadOBJ(obj_path, rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(loaded_obj)

    # The model is a flat plane, so only nodes 0,2 should connect.
    points = [(0,0,1), (0,0,-10), (0,2,0)]
    height = 1.7
    VG = VisibilityGraphAllToAll(bvh, points, height)

    print(VG.CompressToCSR())


.. testoutput::
    :options: +NORMALIZE_WHITESPACE

    (0, 2)        2.236068
    (2, 0)        2.236068

"""

from dhart.geometry import LoadOBJ, CommonRotations
from dhart.raytracer import EmbreeBVH
from dhart.visibilitygraph import VisibilityGraphAllToAll

import dhart

obj_path = dhart.get_sample_model("plane.obj")
loaded_obj = LoadOBJ(obj_path, rotation=CommonRotations.Yup_to_Zup)
bvh = EmbreeBVH(loaded_obj)

# The model is a flat plane, so only nodes 0,2 should connect.
points = [(0,0,1), (0,0,-10), (0,2,0)]
height = 1.7
VG = VisibilityGraphAllToAll(bvh, points, height)

print(VG.CompressToCSR())