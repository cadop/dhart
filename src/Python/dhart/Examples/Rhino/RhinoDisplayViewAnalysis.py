import rhinoscriptsyntax as rs
import json
import scriptcontext as sc
import Rhino
from colorsys import hsv_to_rgb, rgb_to_hsv
from System.Drawing import Color
from Rhino.Display import PointStyle, ColorHSV


point_style = PointStyle.Circle
Point3d = Rhino.Geometry.Point3d
color = Color.Cyan
radius = 7

class json_graph:
    def __init__(self, nodes, edges):
        self.nodes = nodes
        self.edges = edges

    def getEdges(self):
        return [
            (self.nodes[edge[0]],
            self.nodes[edge[1]])
            for edge in self.edges
        ]

def decode_json(filepath):
    try:
        with open(filepath) as in_path:
            json_dict = json.load(in_path)
    except:
        print("FAIL")
        exit()

    return json_dict

def SetupDisplay():
    # Check to see if the display was created yet, if not make a new one
    if "display2" not in sc.sticky:
        print('display Not in sticky')
        sc.sticky["display2"] = Rhino.Display.CustomDisplay(True)# Get the display from sticky context then clear it
    
    display = sc.sticky["display2"]
    display.Clear()
    # display.Enabled = False
    return display

def calcColors(Scores):
    Colors = []
    max_v = max(Scores)
    min_v = min(Scores)
    print('maxmin')
    print(max_v)
    print(min_v)
    print('')
    for point in Scores:
        #scaling the point set
        try: normalized = (point-min_v)/(max_v-min_v)
        except: normalized = 0
        color= colorbar(normalized)
        Colors.append(color)
    return Colors

def colorbar(val):
    
    r = min(max(0, 1.5-abs(1-4*(val-0.5))),1)
    g = min(max(0, 1.5-abs(1-4*(val-0.25))),1)
    b = min(max(0, 1.5-abs(1-4*val)),1)            #conver to hsv?
    tmp_color = rgb_to_hsv(r*255,g*255,b*255)
    return ColorHSV(tmp_color[0],tmp_color[1],tmp_color[2])
    #return (tmp_color[0],tmp_color[1],tmp_color[2])


json_path = "H:\HumanMetrics\Codebase\HumanFactors\dhart\dhart\Examples\out_vg_score.json"
push_off = 120


in_json = decode_json(json_path)
jg = json_graph(in_json["nodes"], in_json["edges"])
colors = calcColors(in_json["vg_count"])

display = SetupDisplay()

for node, scored_color in zip(jg.nodes, colors):
    node_to_add = Point3d(node[0], node[1], node[2]+push_off)
    display.AddPoint(node_to_add, scored_color, point_style, radius)
