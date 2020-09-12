import time
import math
from hfea.utils import graphCreator as gc
from hfea.utils import geomUtils as gu
from hfea.utils import graphData as gd
from hfea.utils import rayUtils as ru

def runGC():

    # start_location = (-30,0,20)
    start_location = (-1,-6,660)
    # start_location = (919,374,577+5)
    x_offset = 10
    y_offset = 10
    # x_offset = 1
    # y_offset = 1
    height = 70
    upstep = 5
    downstep = 5
    upslope = 40
    downslope = 1
    cross_slope = 80
    width = 0
    max_connection = 2
    max_nodes = 30000

    # start_location = (2519,614,661)
    # spacing = (25, 25, 70)
    # x_offset,y_offset,height = spacing
    # max_nodes = 50000

    graphParams = { 'x_offset': x_offset, 'y_offset': y_offset, 'height': height, 'upstep': upstep,
                'downstep': downstep, 'upslope':upslope, 'downslope': downslope, 'cross_slope':cross_slope ,
                'width': width, 'max_connection': max_connection, 'max_nodes' : max_nodes  }
    
    graphParams['ground_offset'] = 0.8
    graphParams['min_connection'] = 8

    geom_bvh = gu.get_bvh('models/Weston_meshed_no-ngon.obj', True)

    s = time.time()
    graph = gc.buildNetwork(start_location,geom_bvh,graphParams)
    print(len(graph.keys()))
    print('Time: ', time.time() - s)

    # dups = []
    # for n1 in graph.keys():
    #     for n2 in graph.keys():
    #         if n1 == n2: continue
    #         if (abs(n1[0]-n2[0])<5) and (abs(n1[1]-n2[1])<5) and (abs(n1[2]-n2[2])<5):
    #             dups.append((n1,n2))
    #             break

    # print(dups)

    gd.storeDefaultGraphData('c_test',graph)

    print('Time: ', time.time() - s)

runGC()