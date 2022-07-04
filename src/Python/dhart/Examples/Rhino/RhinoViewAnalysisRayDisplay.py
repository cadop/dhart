import rhinoscriptsyntax as rs
import json
import scriptcontext as sc
import Rhino
from colorsys import hsv_to_rgb, rgb_to_hsv
from System.Drawing import Color
from Rhino.Display import PointStyle, ColorHSV


point_style = PointStyle.Pin
Point3d = Rhino.Geometry.Point3d
Line = Rhino.Geometry.Line
color = Color.Cyan
radius = 4

def decode_json(filepath):
    with open(filepath) as in_path:
        json_dict = json.load(in_path)


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

json_path = "H:\\HumanMetrics\\Codebase\\DHARTAPI\\dhart\\dhart\\Examples\\view_analysis_directions.json"
push_off = 5
display = SetupDisplay()

in_json = decode_json(json_path)
end_points = in_json["dirs"]
print(len(end_points))

origin = Point3d(0,0,0)
for point in end_points:
    rhino_point = Point3d(point[0], point[1], point[2])
    display.AddPoint(rhino_point)



