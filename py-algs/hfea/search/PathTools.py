import math
import multiprocessing
import os
import platform
import sys
import time
from time import sleep
import imp
import itertools

import numpy as np
import scipy.sparse as sp

try: 
    import dijkstra_mp64 as MPSearch
    import dijkstra_mp64_large as MPSearchL
except:
    from . import dijkstra_mp64_large as MPSearchL
    from . import dijkstra_mp64 as MPSearch
 

dotcount = 0

lil_matrix = sp.lil_matrix
csr_matrix = sp.csr_matrix
sys.setrecursionlimit(1000000)
os.chdir(os.getcwd())

large_graph = False

if platform.architecture()[0] == '32bit':
    low_bit = True
    print("32 bit install detected, expect degraded performance")
else:
    low_bit = False
    print("64 bit install detected, Using Multiprocessing")

def artificialRelativeImport(module, filename):
    """ Imports a file from a different directory """
    this_path = os.path.dirname(os.path.realpath(__file__))
    print(this_path)
    root_path = this_path.split("\\")[:-1]
    DB_list = root_path + [module, filename]
    DB_path = DB_list[0]
    for p in DB_list[1:]:
        DB_path += "\\" + p

    return imp.load_source(filename, DB_path)


def get_path(Pr, i, j):
    path = [j]
    k = j
    pap = path.append
    while Pr[i, k] != -9999:
        pap(Pr[i, k])
        k = Pr[i, k]
    return path[::-1]

def get_paths(Pr, ijs):
    """Construct paths from a predecessor matrix and array of [start,end]
        paths then return as a single list."""
    path = [get_path(Pr, ij[0], ij[1]) for ij in ijs]
    return list(itertools.chain.from_iterable(path))

def graphSearch(graph,num_core=-1):
    """Performs a graph search from every node to every other node."""

    # dist, pred = sp.csgraph.shortest_path(
    # graph, method="D", directed=True, return_predecessors=True)
    if num_core == -1:
        numCore = multiprocessing.cpu_count()
        if numCore > 10: numCore = numCore-2
        elif numCore == 2: pass
        else: numCore = numCore-1
    
    else: 
        numCore = num_core

    global low_bit
    if not low_bit:
        if not large_graph:
            print("searching without large graph")
            dist, pred = MPSearch.multiSearch(graph, numCore)
        else:
            print("searching with large graph")
            dist, pred = MPSearchL.multiSearch(graph, numCore)
    else:
        dist, pred = sp.csgraph.shortest_path(
            graph, method="D", directed=True, return_predecessors=True)
    return dist, pred


def getDistAndPred(graph,node_map, node_lookup, dbName, cost_type = "", directory="", forceGen=False):
    """Generate a CSR Representation of graph, then a Predecessor and Distance
       Matrix.

       If any of these matricies exist at runtime, they will be loaded from
       disk instead of generating new ones. Newly generated matricies will
       be saved to /graphs and will benamed by dbName."""

    global low_bit

    # Construct paths from dbname
    if directory=="":
        directory = "graphs"
    FWdir = directory + "\\" + dbName + cost_type
    FWdirNPY = FWdir + ".npy"
    predDir = directory + "\\" + dbName + cost_type + "_pred"
    predDirNPY = predDir + ".npy"
    graph_path = directory + "\\" + dbName + cost_type + "_sparse.npz"

    # Create the /graphs folder if it doesn't exist
    if not os.path.exists(directory):
        os.makedirs(directory)

    # Try to load everything from file
    try:
        if forceGen:
            raise IOError  # Cheap way to force generation if this is true
        shortPath = np.load(FWdirNPY)
        predecessors = np.load(predDirNPY)
        graphAr = sp.load_npz(graph_path)

    # if they were not stored, generate them here
    except IOError:
        try:
            # Generate CSR and Graphs
            print("Generating CSR...")

            # Check to see if we can use quick 64 bit gen methods
            if not low_bit:
                print('Not low bit')
                graphAr = genCSR(graph,node_map,node_lookup)
                print("Stored Scipy Graph")
                sp.save_npz(graph_path, graphAr)
                
                #check if the data is too large for memory, then use CSRlil method
                datashape = np.shape(graphAr)
                X_size = datashape[0] * datashape[1]

                
                if X_size > 260000000:
                    print("size of data too big for normal MP: ",X_size)

                    global large_graph 
                    large_graph = True

            else:
                print('CSR LIL')
                graphAr = genCSRLil(graph) # this will probably fail until node_map is passed
                print("Stored Scipy Graph")
                sp.save_npz(graph_path, graphAr)
            print("Generating dist and predecessor..")
            try:
                shortPath, predecessors = graphSearch(graphAr, num_core=-1)
            except Exception as e:
                print("Search Failed:")
                print(e)
            # Save graphs
            print("Saving Work to " + directory + "...")
            sp.save_npz(graph_path, graphAr)
            np.save(FWdir, shortPath)
            np.save(predDir, predecessors)
        except IOError as c:
            print("Excepted!")
            print(c)
            sleep(100)

    return graphAr, shortPath, predecessors


def genCSRLil(graph):
    """ Create A CSR representation of the given graph and return as a
        numpy array.

        This is notably slower than genCSR but can be run on 32-bit
        installations of python.
    """
    start_timer = time.time()
    key = list(graph.keys())
    graphAr = lil_matrix((len(key), len(key)))

    for x in range(len(key)):
        value = graph.get(key[x])
        for y in range(len(value)):
            try:
                z = key.index(value[y][0])
            except ValueError:
                continue
            else:
                graphAr[x, z] = value[y][1]

    g = graphAr.tocsr()
    print("Time to calculate CSR LiL: ", time.time() - start_timer)

    return g

def genCSR(graph,node_map,node_lookup):
    """ Generate a CSR matrix from the given graph dictionary."""

    # Initialize variables
    start_timer = time.time()
    data = graph
    nodes = list(data.keys())
    weight = []
    row_idx = []
    col_idx = []

    # Quickly generate a hashtable to find nodeid from (x,y,z) tuple
    # gen = ((nodes[i], i) for i in range(len(nodes))) #maybe not needed
    # lookup = dict(gen)

    #switch the key:value, with value being index 
    lookup = node_lookup

    # Loop through and populate each list
    for parent in nodes:
        for child in data[parent]:
            n1 = lookup[parent]
            n2 = lookup[child[0]] 
            w = child[1]
            row_idx.append(n1)
            col_idx.append(n2)
            weight.append(w)

    # sleep(20)
    # Turn lists into arrays
    weight = np.array(weight)
    row_idx = np.array(row_idx)
    col_idx = np.array(col_idx)
    print('Creating CSR')
    # Calculate shape of matrix, then create CSR
    shape = (len(node_map.keys())+1, len(node_map.keys())+1)
    graph_csr = csr_matrix((weight, (row_idx, col_idx)), shape=shape)
    print("Time to calculate CSR: ", time.time() - start_timer)

    return graph_csr

def updateDisplay(increment, label):
    """
    Clear the screen and print a progressbar at _increment_ percent
    with the given label.

    Use sparingly, cls has a performance cost.
    """

    # Clear the screen
    os.system("cls")

    global dotcount
    dotcount += 1
    # Calculate how many of each object we will need
    dots = "." * ((dotcount % 3) + 1)
    spaces = " " * (100 - increment)
    bars = "|" * increment

    # Print
    print(label + dots)
    progress_bar = "[" + bars + spaces + "]" + str(increment) + "%"
    print(progress_bar)


def getPercentComplete(part, total):
    return int(math.ceil((float(part) / float(total)) * 100))



def getXYZ(jogPaths, key):
    """ Convert node ids to (x,y,z) tuples and return as a list of lists"""
    cordPath = []
    xI = 0
    yI = 0

    # Figure out how many paths we will need to generate
    num_paths = len(jogPaths)

    progress = 0

    for x in jogPaths:
        route = list((key[y] for y in x))
        cordPath.append(route)
    return cordPath


if __name__ == '__main__':
    multiprocessing.freeze_support()
