"""
Notes:

seems python is getting 6 significant digits correct for distance
ex: Moving up by 0.001
69.62304
69.62402

The 2 and 4 seems to be error

"""

import graphCreator as gc
import geomUtils as gu

import time

import graphData as gd

import rayUtils as ru
import math

def normal_round(n,d=0):
    n= n* (10**d)
    if n - math.floor(n) < 0.5:
        return math.floor(n) / (10**d)
    return math.ceil(n) / (10**d)

def trailing_round(x,d=0):
    s = str(x)
    if '.' not in s: return x
    s_int,s_float = s.split('.')
    i_len = len(s_int) # length of integer
    f_len = len(s_float) # length of float 
    precision = f_len - d # number of digits to remove from end
    for i in range(precision):
        x = normal_round(x,f_len-(i+1))
    return x

def trunc(x, p=6):
    s = str(x)
    s_len = len(s)-1 # length without decimal
    if s_len <=p: return x # if the number is within the allowed precision
    if '.' in s: x = float(s[0:p+1])
    else: x = int(s[0:p])
    return x

def rounding_tests():
    geom_bvh = gu.get_bvh('models/Weston_meshed_no-ngon.obj', True)
    prev = 0
    for i in range(100):
        p1 = (999,94,667+70+(i*0.001))
        dir = (0,0,-1)
        dist = ru.cast_ray(geom_bvh, p1, dir, pos=False )
        # if round(p1[2]- dist,3) != prev:
        #     print(round(p1[2]- dist,2))
        #     prev = round(p1[2]- dist,2)
        print('Point: ',p1[2],' Distance: ', dist)
        print('Trunc Dist: ', trunc(dist))
        print(p1[2]- dist)
        print(p1[2]- trunc(dist))
        print(p1[2]- round(dist,2))
        print(round(p1[2]- dist,2) )
        print(trailing_round( p1[2]- dist,2 ) )
        print(normal_round( p1[2]- dist,2 ) )
        print()

    return 

def run():

    geom_bvh = gu.get_bvh('models/Weston_meshed_no-ngon.obj', True)

    # start_location = (-30,0,20)
    start_location = (-1,-6,660)
    x_offset = 20
    y_offset = 20
    # x_offset = 1
    # y_offset = 1
    height = 70
    upstep = 20
    downstep = 20
    upslope = 40
    downslope = 1
    cross_slope = 80
    width = 0
    max_connection = 1
    max_nodes = 50000

    # start_location = (2519,614,661)
    # spacing = (25, 25, 70)
    # x_offset,y_offset,height = spacing
    # max_nodes = 50000

    graphParams = { 'x_offset': x_offset, 'y_offset': y_offset, 'height': height, 'upstep': upstep,
                'downstep': downstep, 'upslope':upslope, 'downslope': downslope, 'cross_slope':cross_slope ,
                'width': width, 'max_connection': max_connection, 'max_nodes' : max_nodes  }
    
    #geom_bvh = gu.get_bvh('models/energy_blob_zup.obj')
    geom_bvh = gu.get_bvh('models/Weston_meshed_no-ngon.obj', True)
    #geom_bvh = gu.get_bvh('models/Weston_Analysis_z-up.obj')

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

    #gd.storeDefaultGraphData('c_test',graph)

    print('Time: ', time.time() - s)

run()
# rounding_tests()