"""

In this example we generate a graph. 

.. testcode::

    import numpy

    from dhart.geometry import LoadOBJ, CommonRotations
    from dhart.raytracer import EmbreeBVH
    from dhart.graphgenerator import GenerateGraph
    from dhart.viewanalysis import SphericalViewAnalysisAggregate, AggregationType

    import dhart

    obj_path = dhart.get_sample_model("plane.obj")

    obj = LoadOBJ(obj_path, rotation=CommonRotations.Yup_to_Zup)
    bvh = EmbreeBVH(obj)

    start_point = (-1, -6, 1623.976928)

    spacing = (0.5, 0.5, 0.5)
    max_nodes = 500

    graph = None
    graph = GenerateGraph(bvh, start_point, spacing, max_nodes, cores=-1)

    if graph is None:
        print("No graph generated!")
        exit()

    csr_graph = graph.CompressToCSR()
    nodes = graph.getNodes()

    view_analysis = SphericalViewAnalysisAggregate(bvh, nodes, 10000, 1.7, agg_type=AggregationType.SUM)

    for va in view_analysis.array:
        print(round(va, 5))

.. testoutput:: 



"""

import numpy

from dhart.geometry import LoadOBJ, CommonRotations
from dhart.raytracer import EmbreeBVH
from dhart.graphgenerator import GenerateGraph
from dhart.viewanalysis import SphericalViewAnalysisAggregate, AggregationType

import dhart

obj_path = dhart.get_sample_model("plane.obj")

obj = LoadOBJ(obj_path, rotation=CommonRotations.Yup_to_Zup)
bvh = EmbreeBVH(obj)

start_point = (-1, -6, 1623.976928)

spacing = (0.5, 0.5, 0.5)
max_nodes = 500

graph = None
graph = GenerateGraph(bvh, start_point, spacing, max_nodes, cores=-1)

if graph is None:
    print("No graph generated!")
    exit()

csr_graph = graph.CompressToCSR()
nodes = graph.getNodes()

view_analysis = SphericalViewAnalysisAggregate(bvh, nodes, 10000, 1.7, agg_type=AggregationType.SUM)

for va in view_analysis.array:
    print(round(va, 5))
