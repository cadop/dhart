import tkinter as tk
from tkinter import filedialog
import numpy

from humanfactorspy.geometry import LoadOBJ, CommonRotations
from humanfactorspy.raytracer import EmbreeBVH
from humanfactorspy.graphgenerator import GenerateGraph
from humanfactorspy.viewanalysis import SphericalViewAnalysisAggregate, AggregationType


# obj_path = "H:\\HumanMetrics\\Codebase\\HumanFactors\\out\\install\\x64-Debug\\Example Models\\plane.obj"
obj_path = None

if obj_path is None:
    root = tk.Tk()
    root.withdraw()
    obj_path = filedialog.askopenfilename()

obj = LoadOBJ(obj_path, rotation=CommonRotations.Yup_to_Zup)
bvh = EmbreeBVH(obj)

start_point = (-1, -6, 1623.976928)

spacing = (0.5, 0.5, 0.5)
max_nodes = 500

graph = GenerateGraph(bvh, start_point, spacing, max_nodes, cores=-1)

if not graph:
    print("No graph generated!")
    exit()

csr_graph = graph.CompressToCSR()
nodes = graph.getNodes()

view_analysis = SphericalViewAnalysisAggregate(
    bvh, nodes, 10000, 1.7, AggregationType.SUM
)

for va in view_analysis.array:
    print(round(va, 5))
