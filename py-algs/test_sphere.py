import numpy as np
from numpy.lib import recfunctions as rfn

import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

from humanfactorspy.viewanalysis import SphericallyDistributeRays
from humanfactorspy.viewanalysis import SphericalViewAnalysis

from hfea.utils import geomUtils as gu
from hfea.utils import rayUtils as ru

def test_intersect():

    from humanfactorspy.geometry import LoadOBJ, CommonRotations, ConstructPlane
    from humanfactorspy.raytracer import EmbreeBVH, Intersect

    dirs = SphericallyDistributeRays(200, upward_fov=10,downward_fov=10)
    print(len(dirs))

    # loaded_obj = ConstructPlane()
    # loaded_obj.Rotate(CommonRotations.Yup_to_Zup)
    # bvh = EmbreeBVH(loaded_obj, True)
    bvh = gu.get_bvh('models/Weston_meshed_no-ngon.obj')

    # origins = [(0,0,x) for x in range(0,5)]
    n1 = (0,0,0)
    n2 = (0,0,5)

    n1 = (1219,84,625.5+40)
    n2 = (-351,-26,625.5+40)
    origins = [ n1, n2]
    hit_point = Intersect(bvh, origins, (0.0,0.0,-1.0))
    res = rfn.structured_to_unstructured(hit_point.array)
    print(res)

    dist_array = ru.cast_ray_multi(bvh, origins, (0,0,-1))
    print(rfn.structured_to_unstructured(hit_point.array))


def run():
    ray_count = 2000
    dirs = SphericallyDistributeRays(ray_count, upward_fov=10,downward_fov=10)

    print(len(dirs))

    geom_bvh = gu.get_bvh('models/Weston_meshed_no-ngon.obj',True)

    height = 40
    n1 = (1219,84,625.5)
    n2 = (-351,-26,625.5)

    n1 = (1399,534,626+height)

    n1_h = (1219,84,625.5+height)
    n2_h = (-351,-26,625.5+height)

    nodes = [n1,n2_h]

    scores = np.empty((len(dirs),len(nodes)))
    for idx, dir in enumerate(dirs):
        scores[idx] = ru.cast_ray_multi(geom_bvh, nodes, dir)['distance']

    scores = np.amin(scores, axis=0)
    print(scores)

    nodes = [n1, n2]

    scores = SphericalViewAnalysis(geom_bvh, nodes, ray_count, height,
                                   upward_fov = 10, downward_fov=10)
    scores = np.amin(scores['distance'], axis=1)

    print(scores)

    # Plot the graph in 3D
    fig = plt.figure()
    ax = Axes3D(fig)
    ax.view_init(azim=-123, elev=15)

    ax.scatter(dirs[:,0], dirs[:,1], dirs[:,2])
    plt.show()

    return 

# test_intersect()
run()