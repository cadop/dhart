"""

In this example we generate a graph. 

.. testcode::

    from humanfactorspy.geometry import LoadOBJ, CommonRotations
    from humanfactorspy.raytracer import EmbreeBVH
    from humanfactorspy.graphgenerator import GenerateGraph
    from humanfactorspy.viewanalysis import SphericalViewAnalysisAggregate, AggregationType

    import humanfactorspy

    obj_path = humanfactorspy.get_sample_model("plane.obj")

    obj = LoadOBJ(obj_path, rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(obj)

    start_point = (-1, -6, 1623.976928)

    spacing = (0.5, 0.5, 0.5)
    max_nodes = 500

    graph = GenerateGraph(bvh, start_point, spacing, max_nodes, cores=-1)

    csr_graph = graph.CompressToCSR()
    nodes = graph.getNodes()

    print(len(nodes))
    print(nodes[0:3])


.. testoutput:: 

    594
    [(-1. , -6. ,  0., 0, 0) (-1.5, -6.5, -0., 0, 1) (-1.5, -6. , -0., 0, 2)]

"""

from humanfactorspy.geometry import LoadOBJ, CommonRotations
from humanfactorspy.raytracer import EmbreeBVH
from humanfactorspy.graphgenerator import GenerateGraph
from humanfactorspy.viewanalysis import SphericalViewAnalysisAggregate, AggregationType

import humanfactorspy

obj_path = humanfactorspy.get_sample_model("plane.obj")

obj = LoadOBJ(obj_path, rotation=CommonRotations.Yup_to_Zup)
bvh = EmbreeBVH(obj)

start_point = (-1, -6, 1623.976928)

spacing = (0.5, 0.5, 0.5)
max_nodes = 500

graph = GenerateGraph(bvh, start_point, spacing, max_nodes, cores=-1)

csr_graph = graph.CompressToCSR()
nodes = graph.getNodes()

print(len(nodes))
print(nodes[0:3])
