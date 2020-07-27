"""

In this example we generate a graph on a plane and print out the graph as a CSR matrix
consisting of the node ides and the edge cost (which is distance in this case).

.. testcode::

    from humanfactorspy.geometry import LoadOBJ, CommonRotations
    from humanfactorspy.raytracer import EmbreeBVH
    from humanfactorspy.graphgenerator import GenerateGraph
    import humanfactorspy

    obj_path = humanfactorspy.get_sample_model("plane.obj")

    obj = LoadOBJ(obj_path, rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(obj)

    start_point = (-1, -6, 1623.976928)

    spacing = (0.5, 0.5, 0.5)
    max_nodes = 5

    graph = GenerateGraph(bvh, start_point, spacing, max_nodes, cores=1)

    csr_graph = graph.CompressToCSR()
    print(csr_graph)


.. testoutput:: 
    :options: +NORMALIZE_WHITESPACE

    (0, 1)    0.70710677
    (0, 2)    0.5
    (0, 3)    0.70710677
    (0, 4)    0.5
    (0, 5)    0.5
    (0, 6)    0.70710677
    (0, 7)    0.5
    (0, 8)    0.70710677
    (1, 0)    0.70710677
    (1, 2)    0.5
    (1, 4)    0.5
    (1, 9)    0.70710677
    (1, 10)   0.5
    (1, 11)   0.70710677
    (1, 12)   0.5
    (1, 13)   0.70710677
    (2, 0)    0.5
    (2, 1)    0.5
    (2, 3)    0.5
    (2, 4)    0.70710677
    (2, 5)    0.70710677
    (2, 10)   0.70710677
    (2, 11)   0.5
    (2, 14)   0.70710677
    (3, 0)    0.70710677
    (3, 2)    0.5
    (3, 5)    0.5
    (3, 11)   0.70710677
    (3, 14)   0.5
    (3, 15)   0.70710677
    (3, 16)   0.5
    (3, 17)   0.70710677
    (4, 0)    0.5
    (4, 1)    0.5
    (4, 2)    0.70710677
    (4, 6)    0.5
    (4, 7)    0.70710677
    (4, 12)   0.70710677
    (4, 13)   0.5
    (4, 18)   0.70710677

"""

from humanfactorspy.geometry import LoadOBJ, CommonRotations
from humanfactorspy.raytracer import EmbreeBVH
from humanfactorspy.graphgenerator import GenerateGraph
import humanfactorspy

obj_path = humanfactorspy.get_sample_model("plane.obj")

obj = LoadOBJ(obj_path, rotation=CommonRotations.Yup_to_Zup)
bvh = EmbreeBVH(obj)

start_point = (-1, -6, 1623.976928)

spacing = (0.5, 0.5, 0.5)
max_nodes = 5

graph = GenerateGraph(bvh, start_point, spacing, max_nodes, cores=1)

csr_graph = graph.CompressToCSR()
print(csr_graph)