import tkinter as tk
from tkinter import filedialog
import numpy
#import json

from dhart.geometry import LoadOBJ, CommonRotations
from dhart.raytracer import EmbreeBVH
from dhart.graphgenerator import GenerateGraph
from dhart.viewanalysis import SphericalViewAnalysisAggregate, AggregationType, SphericalViewAnalysis, SphericallyDistributeRays

# Try to load ujson since it's really fast
try:
    import ujson as json
except:
    import json as json
    print("Ujson not detected. Using standard, slower python json writer ")

json_dict = {}


distance = 20
directions = SphericallyDistributeRays(int(20000), 70, 20)
dir_list = directions.array.tolist()


hit_points = [
    (
        dir[0] * distance,
        dir[1] * distance,
        dir[2] * distance
    )
    for dir in dir_list
]

json_dict["dirs"] = hit_points
print("Writing to file")
with open("view_analysis_directions.json", "w") as out_json:
    json.dump(json_dict, out_json)