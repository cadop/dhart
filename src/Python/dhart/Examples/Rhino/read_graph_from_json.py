import rhinoscriptsyntax as rs
import json
import scriptcontext as sc
import Rhino
from colorsys import hsv_to_rgb, rgb_to_hsv
from System.Drawing import Color
from Rhino.Display import PointStyle, ColorHSV, ColorLAB



point_style = PointStyle.Asterisk
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
    
def lerp(t, min, max):
    return (1 - t) * min + t * max

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
    filtered_scores = [score for score in Scores if score>0]
    max_v = max(filtered_scores)
    min_v = min(filtered_scores)
    print('maxmin')
    print(max_v)
    print(min_v)
    print('')
    if (max_v == 0):
        print("Max is zero!")
        exit()
    elif(max_v - min_v == 0):
        print("All values are the same!")
        return [colorbar(1.0)] * len(Scores)
        
    return [
            colorbar((point-min_v)/(max_v-min_v))
            if point > 0
            else None
            for point in Scores
    ]


def colorbarlab(val):
    # 0-1 scale
    L = 0.1
    A = max((val-0.5)*2, 0)
    B = min(val*2.0, 1)
    
    return ColorLAB(L,1,0,0)

def colorbar(val):
    r = min(max(0, 1.5-abs(1-4*(val-0.5))),1)
    g = min(max(0, 1.5-abs(1-4*(val-0.25))),1)
    b = min(max(0, 1.5-abs(1-4*val)),1)            #conver to hsv?
    tmp_color = rgb_to_hsv(r*255,g*255,b*255)
    return ColorHSV(tmp_color[0],tmp_color[1],tmp_color[2])

def DrawPoints(nodes, colors, point_style, radius):
    for node, scored_color in zip(jg.nodes, colors):
        if scored_color == None:
            continue
            
        node_to_add = Point3d(node[0], node[1], node[2]+push_off)
        display.AddPoint(node_to_add, scored_color, point_style, radius)

display = SetupDisplay()
json_path = "H:\HumanMetrics\Codebase\DHARTAPI\dhart\dhart\Examples\out_graph.json"
push_off = 100

in_json = decode_json(json_path)
jg = json_graph(in_json["nodes"], [])
colors = calcColors(in_json["vg_count"])
# colors = [Color.Red] * len(jg.nodes)

for node, scored_color in zip(jg.nodes, colors):
    if scored_color == None:
        continue
        scored_color = Color.White
    node_to_add = Point3d(node[0], node[1], node[2]+push_off)
    display.AddPoint(node_to_add, scored_color, point_style, radius)
