import graphCreator as gc
import geomUtils as gu

import time

import graphData as gd

def run():

    start_location = (-30,0,20)
    #start_location = (-1,-6,660)
    x_offset = 10
    y_offset = 10
    x_offset = 1
    y_offset = 1
    height = 180
    upstep = 30
    downstep = 70
    upslope = 45
    downslope = 45
    cross_slope = 80
    width = 0
    max_connection = 1
    max_nodes = 150000

    graphParams = { 'x_offset': x_offset, 'y_offset': y_offset, 'height': height, 'upstep': upstep,
                'downstep': downstep, 'upslope':upslope, 'downslope': downslope, 'cross_slope':cross_slope ,
                'width': width, 'max_connection': max_connection, 'max_nodes' : max_nodes  }
    
    geom_bvh = gu.get_bvh('models/energy_blob_zup.obj')
    # geom_bvh = gu.get_bvh('models/Weston_comparison.obj', True)
    #geom_bvh = gu.get_bvh('models/Weston_Analysis_z-up.obj')

    s = time.time()
    graph = gc.buildNetwork(start_location,geom_bvh,graphParams)
    print(len(graph.keys()))
    print('Time: ', time.time() - s)


    #gd.storeDefaultGraphData('c_test',graph)

    print('Time: ', time.time() - s)

run()