import operator
import os
import platform
import sqlite3
import time
import itertools
import shutil
from collections import deque
from math import sqrt
from time import sleep
from collections import defaultdict
import json

if platform.architecture()[0] == "32bit":
    low_mem = True
else:
    low_mem = False
dbname = "test"
pathname = None


def initDB(inputname="test", ret_path = False):
    """Open the database with the specified name. Return True if a new
       DB needed to be created.
    """
    global dbname
    global pathname
    # Get the path to this python file and change working directory
    dir_path = os.path.dirname(os.path.realpath(__file__))
    os.chdir(dir_path)

    # Get the path to the DB for the given filename
    dbname = "sql\\" + inputname + ".db"
    pathname = inputname

    # Check to see if our database folder exists
    if not os.path.exists("sql"):
        os.mkdir("sql")

    # If the db has not yet been created, init all the tables
    if not os.path.isfile(dbname):
        dbname = dbname
        createTables()
        return True

    if ret_path:
        return dir_path +"\\" +dbname
    return False


def openConn(Danger=False):
    """ Open a new connection to the DB, then return it"""
    global dbname
    global low_mem

    # Change the filepath
    dir_path = os.path.dirname(os.path.realpath(__file__))
    os.chdir(dir_path)

    # Open a new conn so we can set up our inserts
    conn = sqlite3.connect(dbname,detect_types=sqlite3.PARSE_DECLTYPES|sqlite3.PARSE_COLNAMES)
    c = conn.cursor()

    # No journal is faster, but fraught with peril.
    # Any crash has a high chance of corrupting the DB.
    if True:
        c.execute("PRAGMA journal_mode = OFF")
    else:
        c.execute("PRAGMA journal_mode = MEMORY")

    c.execute("PRAGMA synchronous = off")

    # Use a larger cache if we can
    c.execute("PRAGMA cache_size = -100000")
    c.execute("PRAGMA threads = 16")
    c.close()
    return conn


def createTables():
    """ Create all tables that do not yet exist for the DB. """
    # NODE TABLE
    conn = openConn()
    c = conn.cursor()
    c.execute(
        """CREATE TABLE IF NOT EXISTS Nodes
                (nodeID INTEGER PRIMARY KEY AUTOINCREMENT,
                nodeX REAL,
                nodeY REAL,
                nodeZ REAL,
                terrainType TEXT DEFAULT "Unknown"
                )"""
    )

    # EDGE TABLE
    c.execute(
        """CREATE TABLE IF NOT EXISTS Edges
                (edgeID INTEGER PRIMARY KEY AUTOINCREMENT,
                parentID INTEGER,
                childID INTEGER,
                cost REAL)
                """
    )

    # LOCATION TABLE
    c.execute(
        """CREATE TABLE IF NOT EXISTS Location
                (locID INTEGER PRIMARY KEY AUTOINCREMENT,
                name TEXT,
                Lat REAL,
                Long REAL)
                """
    )

    # User Parameters TABLE
    c.execute(
        """CREATE TABLE IF NOT EXISTS UserParams
                (paramID INTEGER PRIMARY KEY AUTOINCREMENT,
                paramKey TEXT,
                paramVer INTEGER,
                args json)
                """
    )

    # BUILDING TABLE
    c.execute(
        """CREATE TABLE IF NOT EXISTS Buildings
                (buildID INTEGER,
                class TEXT,
                volume INTEGER,
                sqft INTEGER,
                height INTEGER,
                exitnode INTEGER)
                """
    )

    # PATHS TABLE
    c.execute(
        """CREATE TABLE IF NOT EXISTS Paths
                (pathID INTEGER PRIMARY KEY AUTOINCREMENT,
                rank INTEGER)
                """
    )
    
    # PATHS QUEUE TABLE
    c.execute(
        """ CREATE TABLE IF NOT EXISTS Path_Queue(
                A int,
                B int,
                type Text,
                UNIQUE(A,B,type))
        """
    )

    # SHORT_PATHS TABLE
    c.execute(
        """CREATE TABLE IF NOT EXISTS ShortPaths
                (pathID INTEGER PRIMARY KEY,
                A INTEGER,
                B INTEGER,
                type TEXT,
                lookup TEXT)
                """
    )
    # PATH_MEMBERS TABLE
    c.execute(
        """CREATE TABLE IF NOT EXISTS Path_Members
                (pathID INTEGER,
                nodeID INTEGER,
                nodeIndex INTEGER)
                """
    )

    # ShortPathMembers TABLE
    c.execute(
        """CREATE TABLE IF NOT EXISTS ShortPathMembers
                (pathID INTEGER,
                nodeID INTEGER,
                nodeIndex INTEGER,
                weight_to_next INTEGER)
                """
    )
    # Picklepaths TABLE
    c.execute(
        """CREATE TABLE IF NOT EXISTS PicklePaths(
                pathID INTEGER PRIMARY KEY AUTOINCREMENT,
                A VARCHAR,
                B VARCHAR,
                path BLOB
                )"""
    )

    # NODE_PARAM TABLE
    c.execute(
        """CREATE TABLE IF NOT EXISTS Node_Param
                (nodeID INTEGER,
                paramName TEXT,
                paramValue TEXT)
                """
    )

    # LINK_PARAM TABLE
    c.execute(
        """CREATE TABLE IF NOT EXISTS Link_Param
                (linkID INTEGER,
                paramName TEXT,
                paramValue REAL)
                """
    )

    # BUILDING_PARAM TABLE
    c.execute(
        """CREATE TABLE IF NOT EXISTS Building_Param
                (buildID TEXT,
                paramName TEXT,
                paramValue TEXT)
                """
    )

    # Indexes
    c.execute(
        """CREATE INDEX IF NOT EXISTS LinkParamIndex ON Link_Param (
    linkID)"""
    )
    c.execute("CREATE INDEX IF NOT EXISTS SPMID on ShortPathMembers(pathID)")
    c.execute("CREATE INDEX IF NOT EXISTS SPF on Shortpaths(A)")
    closeConn(conn)


# DELETE--------------------------------------------------


def purgeDB():
    """Drop all tables in the DB, then recreate them. Use this to start fresh"""
    conn = openConn()
    c = conn.cursor()

    global dbname

    # Drop all relevant tables
    c.execute("DROP TABLE IF EXISTS Nodes")
    # c.execute('''DROP TABLE IF EXISTS Buildings''') buildings can stay
    c.execute("DROP TABLE IF EXISTS Edges")
    c.execute("DROP TABLE IF EXISTS Location")
    c.execute("DROP TABLE IF EXISTS UserParams")
    c.execute("DROP TABLE IF EXISTS Paths")
    c.execute("DROP TABLE IF EXISTS Path_Members")
    c.execute("DROP TABLE IF EXISTS ShortPathMembers")
    c.execute("DROP TABLE IF EXISTS ShortPaths")
    c.execute("DROP TABLE IF EXISTS Node_Param")
    c.execute("DROP TABLE IF EXISTS Link_Param")
    c.execute("DROP TABLE IF EXISTS Building_Param")
    c.execute("DROP TABLE IF EXISTS PicklePaths")

    # also purge every matrix
    try:
        shutil.rmtree("graphs")
    except:
        pass
    closeConn(conn)
    createTables()


def purgeBuildings():
    """ Drop and recreate the buildings table."""
    conn = openConn()
    c = conn.cursor()

    c.execute(""" DROP TABLE IF EXISTS Buildings """)
    createTables()

    closeConn(conn)


def purgePaths():
    """ Drop and recreate all path tables."""
    conn = openConn()
    c = conn.cursor()

    c.execute(""" DROP TABLE IF EXISTS Paths """)
    c.execute(""" DROP TABLE IF EXISTS Path_Members """)
    c.execute(""" DROP TABLE IF EXISTS PicklePaths """)

    createTables()

    closeConn(conn)


def inputGraph(graph, timing=True):
    """ Stores a dictionary from GraphGenerator in the DB."""

    # Create a hashmap for node row ids before going into sql
    purgeDB()  # If we're inputting, we should purge
    counter = 1

    #get all the nodes that are children
    #This skips a parent that is not a child of any node, but that should not be possible 
    # since it must be a child in order to be added to the todo list in graph generation
    allNodes = set( [  y[0] for x in graph for y in graph[x]] )
    keys = list(allNodes)
    row_ids = {keys[i]: i + 1 for i in range(0, len(allNodes))}
    counter = len(allNodes) + 1

    # keys = list(graph.keys())
    # row_ids = {keys[i]: i + 1 for i in range(0, len(graph))}
    # counter = len(graph) + 1

    links = []
    linksappend = links.append
    # Iterate through node/link pair
    for node, edges in graph.items():

        # Iterate through edges
        parent_id = row_ids[node]
        for edge in edges:

            # Get Weight
            weight = edge[1]

            # Check to see if this child exists. If not, add to id dictionary
            child = edge[0]
            if child in row_ids:
                child_id = row_ids[child]
            else:
                row_ids[child] = counter
                child_id = counter
                counter += 1

            # Add to list of links
            linksappend((parent_id, child_id, weight))

    # Sort the dictionary of nodes by key
    sorted_pairs = sorted(row_ids.items(), key=operator.itemgetter(1))

    # Remove nodes from the node, key pairs
    sorted_nodes = [pair[0] for pair in sorted_pairs]

    # Insert into DB
    conn = openConn()
    c = conn.cursor()

    c.executemany("INSERT INTO Nodes VALUES (null,?,?,?,null)", sorted_nodes)
    c.executemany("INSERT INTO Edges VALUES (null,?,?,?)", links)
    closeConn(conn)


def getDictionary(cost_type=None,retOrphans=False):
    """ 
    Recreate dictionary output from GraphGen, then return it
    if retOrphans is True, it will return the graph including the child nodes that are not parents 
    """
    conn = openConn()
    c = conn.cursor()
    ironpython = True
    # link_tuples = [((link[1], link[2], link[3]), ((link[5], link[6], link[7]), link[8])) for link in c.fetchall()]

    # Get every Node in our DB
    c.execute("SELECT nodeX, nodeY, nodeZ FROM Nodes ORDER BY nodeID")
    nodes = c.fetchall()

    # Get Every Edge in our DB
    if cost_type == "Distance" or cost_type is None:
    # if cost_type is None:
        c.execute("SELECT parentid, childid, cost FROM Edges")
        links = c.fetchall()
    else:
        if not ironpython:
            # Too slow in Ironpython!
            c.execute(
                """SELECT e.parentid, e.childid, lp.paramValue
                        FROM Edges e
                        LEFT JOIN Link_Param lp
                            ON lp.LinkID = e.edgeID
                        WHERE lp.paramName=?""",
                (cost_type,),
            )
            links = c.fetchall()
        else:
            t = time.time()
            # Get a dictionary of (id:parent, child)
            c.execute("SELECT LinkID, paramValue FROM Link_Param WHERE paramName ='"+cost_type+ "' ORDER BY LinkID")
            params = c.fetchall()
            print(time.time() - t, "params")
            t = time.time()
            c.execute("SELECT edgeID, parentID, childID FROM Edges ORDER BY edgeID")
            edges = c.fetchall()
            print(time.time() - t, "edges")
            # Check to see if we need to use a dictionary thing
            if len(edges) == len(params):
                links = [
                    (edges[i][1], edges[i][2], params[i][1])
                    for i in range(0, len(edges))
                ]

            # This gets a little complicatedj
            else:
                # Create a reverse dictionary of links
                links = []
                linksappend = links.append
                # edge_map = {edge[0]: (edge[1], edge[2]) for edge in c.fetchall()}
                edge_map = {edge[0]: (edge[1], edge[2]) for edge in edges}
                
                
                # Iterate through each param, find the link, then append it to list of links
                for i in range(0, len(params)):
                    param = params[i]
                    edge = edge_map[param]
                    linksappend((edge[0], edge[1], param))

    # Initalize dictionary of nodes
    graphdict = {node: [] for node in nodes}

    # Iterate through each link to add it to the respective node

    start = time.time()
    for link in links:

        # Get our parent and child xyz from their indexes
        parent = nodes[link[0] - 1]
        child = nodes[link[1] - 1]
        weight = link[2]

        graphdict[parent].append((child, weight))

    closeConn(conn)
    if cost_type is None or cost_type == "Distance":
        return graphdict
    # If we're requesting a specific graph, only return nodes that have links
    else:
        if retOrphans == False:
            return {k: v for k, v in graphdict.items() if len(v) != 0}
        else:  return graphdict


def addNodes(nodes):
    """ Store a list of nodes in the DB. """
    conn = openConn()
    c = conn.cursor()

    c.executemany("INSERT INTO Nodes VALUES (null,?,?,?,null)", nodes)

    closeConn(conn)


def deleteNodes(nodes):
    """ Deletes all nodes and edges from the DB with the given (x,y,z)"""
    conn = openConn()
    c = conn.cursor()
    d = conn.cursor()
    f = conn.cursor()

    for i in range(len(nodes)):
        c.execute(
            "SELECT nodeID FROM Nodes WHERE nodeX = (?) AND nodeY = (?) AND nodeZ = (?)",
            nodes[i],
        )
        x = c.fetchone()
        d.execute(
            "DELETE FROM Edges WHERE parentID = (?) OR childID = (?)", (x[0], x[0])
        )

        # REMOVES PATHS THAT CONTAINED DELETED NODE
        for y in d.execute(
            "SELECT pathID FROM Path_Members WHERE nodeID = (?)", (x[0],)
        ):
            f.execute("DELETE FROM Path_Members WHERE pathID = (?)", y)
            f.execute("DELETE FROM Paths WHERE pathID = (?)", y)

    c.executemany(
        "DELETE FROM Nodes WHERE nodeX = (?) AND nodeY = (?) AND nodeZ = (?)", nodes
    )

    closeConn(conn)


def setTerrainType(assign_pairs):
    """ Change the terrain type for (x,y,z), terraintype tuples representing nodes."""

    node_map = getHashMap()
    # Unpack the pairs into DB ready tuples with a quick hashmap check
    assignment_tuples = [(pair[0], node_map[pair[1]]) for pair in assign_pairs]

    # Execute the Query
    conn = openConn()
    c = conn.cursor()
    print(assignment_tuples[0])
    c.executemany(
        "UPDATE Nodes SET terrainType = ? WHERE nodeID = ?", assignment_tuples
    )

    closeConn(conn)


def getNodes(terrain="all"):
    """ Returns an list of 3 element tuples (x,y,z) representing nodes
        in the graph.

        If terrain is specified, only return nodes with a matching
        terrain type. Useful for getting all nodes on sidewalks or
        on bikelanes """
    conn = openConn()
    c = conn.cursor()
    out = []

    # Check to see if a terrain type was specified
    if terrain == "all":
        c.execute("SELECT nodeX, nodeY, nodeZ FROM Nodes ORDER BY nodeID")
        # nodes = c.fetchmany()
        nodes = c.fetchall()
        closeConn(conn)
        return nodes

    # If so, search for the type of terrain
    else:
        c.execute(
            "SELECT nodeX, nodeY, nodeZ FROM Nodes WHERE terrainType = (?)", (terrain,)
        )
        nodes = c.fetchall()

        closeConn(conn)
        return nodes


# EDGES---------------------------------------------------------------


def addEdges(inNodes):
    """ Store a list of edges in the form (node_1, node_2, weight) """
    conn = openConn()
    c = conn.cursor()

    c.executemany("INSERT INTO Edges VALUES (null,?,?,?)", inNodes)
    # c.execute("CREATE INDEX path_parent ON Edges(parentID, childID)") Takes
    # too long
    closeConn(conn)


def getEdgeDictionary(name=None):
    """ Create a dictionary of {(parent, child):weight} for each link."""
    # If the name is none, then it doesn't matter
    ironpython = True
    if name is None or name == "Distance":
        # Create hash map with cost
        edge_map = {(edge[0], edge[1]): edge[2] for edge in getEdges()}

    # Hardcode this to get a list of edgeids
    elif name == "ID":
        conn = openConn()
        c = conn.cursor()
        c.execute("SELECT edgeID, parentID, childID FROM Edges")

        edge_map = {(edge[1], edge[2]): edge[0] for edge in c.fetchall()}

        closeConn(conn)
    elif name == "ReverseID":
        conn = openConn()
        c = conn.cursor()
        c.execute("SELECT edgeID, parentID, childID FROM Edges")

        # Get parameters by ID
        edge_map = {edge[0]: (edge[1], edge[2]) for edge in c.fetchall()}
        closeConn(conn)

    else:
        conn = openConn()
        c = conn.cursor()        
        # If it's ironpython, then joins are slow so we need to do the work of the join
        if ironpython:

            print('Getting edges')
            t = time.time()
            # # Get a dictionary of (id:parent, child)
            # c.execute("SELECT LinkID, paramValue FROM Link_Param ORDER BY LinkID")
            # params = c.fetchall()
            # print time.time() - t, "params"
            # t = time.time()
            # c.execute("SELECT edgeID, parentID, childID FROM Edges ORDER BY edgeID")
            # edges = c.fetchall()
            # print time.time() - t, "edges"

            # # Check to see if we need to use a dictionary thing
            # if len(edges) == len(params):
                # edge_map = {
                    # (edges[i][1], edges[i][2]): params[i][1]
                    # for i in xrange(0, len(edges))
                # }

            # # This gets a little complicated
            # else:
                # # Create a reverse dictionary of links
                # edge_dict = {edge[0]: (edge[1], edge[2]) for edge in c.fetchall()}
                # edge_map = {edge_map[param]: param for param in params}
            
            ###***************************    
            # Get a dictionary of (id:parent, child)
            c.execute("SELECT LinkID, paramValue FROM Link_Param WHERE paramName ='"+name+ "' ORDER BY LinkID")
            params = c.fetchall()
            
            print(time.time() - t, "params")
            # print(params)
            t = time.time()
            c.execute("SELECT edgeID, parentID, childID FROM Edges ORDER BY edgeID")
            edges = c.fetchall()
            
            print(time.time() - t, "edges")
            # print(edges)
            # Check to see if we need to use a dictionary thing
            # if len(edges) == len(params):
                # links = [
                    # (edges[i][1], edges[i][2], params[i][1])
                    # for i in xrange(0, len(edges))
                # ]

            # else:
                # # Create a reverse dictionary of links
                # links = []
                # linksappend = links.append
                # edge_map = {edge[0]: (edge[1], edge[2]) for edge in edges}
    
            # Create a reverse dictionary of links
            links = []
            linksappend = links.append
            tmp_map = {edge[0]: (edge[1], edge[2]) for edge in edges}        
            # Iterate through the inverted edge map and find the correct value
            #make an empty  dict for edge_map 
            edge_map = {}
            #make params a dict for easy lookup
            paramDict = dict(params)
            for key in tmp_map:
                edge = tmp_map[key]
                edge_map[edge] = paramDict[key]
            
        else:
            c.execute(
                """SELECT e.parentID, e.childID, p.paramValue
                        FROM Edges e
                        LEFT JOIN Link_Param p
                            ON e.edgeID = p.linkID
                        WHERE p.paramName = (?)""",
                (name,),
            )

            # Same list comp as before
            edge_map = {(edge[0], edge[1]): edge[2] for edge in c.fetchall()}
        closeConn(conn)

    # conn.execute("SELECT  FROM ")
    return edge_map


def deleteEdges(edge_list):
    """ Delete a list of edges matching input node pairs
        (node_1, node_2) """
    conn = openConn()
    c = conn.cursor()

    c.executemany("DELETE FROM Edges WHERE parentID = (?) AND childID = (?)", edge_list)

    closeConn(conn)


def updateEdgeCost(inNodes):
    """ Update the cost of specified edges in form (node_1, node_2, new_cost) """
    conn = openConn()
    c = conn.cursor()

    for x in range(len(inNodes)):
        update = (inNodes[x][2], inNodes[x][0], inNodes[x][1])
        c.execute(
            "UPDATE Edges SET cost = (?) WHERE parentID = (?) AND childID = (?)", update
        )

    closeConn(conn)


def getEdges(inNodes="all"):
    """ Return a list of edges in the form of tuples of (parent, child, cost)"""
    conn = openConn()
    c = conn.cursor()
    if inNodes == "all":
        c.execute("SELECT parentID, childID, cost FROM Edges")
        edges = c.fetchall()
        closeConn(conn)
        return edges


def addJogPaths(path_list, start_end_pairs, cost_type,lookupNames):
    """ Store a list of building to building paths to the database.

        Keyword Args:
        path_list -- a list of lists of (x,y,z) node tuples
        start_end_pairs -- list of tuples conatining the start building
                           and end building for each path of matching
                           index

    """
    # dropPaths(True)

    if cost_type == None:
        path_type = "Distance"
    else:
        path_type = cost_type

    s = time.time()

    # Get Hashmap
    node_map = getHashMap()
    edge_dict = getEdgeDictionary(cost_type)
    # print('Got edges: ')
    # print(edge_dict)
    conn = openConn()
    c = conn.cursor()

    # Get the maximum path id
    c.execute(
        """SELECT pathID
                FROM ShortPaths
                ORDER BY ShortPaths.pathID DESC
                LIMIT 1"""
    )
    potential_id = c.fetchall()
    
    # Check if this fails since there may not be any paths in the DB
    pathid = potential_id[0][0] + 1 if len(potential_id) > 0 else 1

    # Cache these methods for faster appends.
    path_members = []  # Tuples (pathid, nodeid, index)
    path_headers = []  # tuples (pathid, building_A, building_B)

    pathmembersappend = path_members.append
    pathheadersappend = path_headers.append

    # print('iterating')
    # Iterate through each path to precompute our insertion
    for i in range(0, len(path_list)):
        # Create a path header of (pathid, start_building, end_building)
        building_tuple = start_end_pairs[i]
        path = path_list[i]
        pathheadersappend((pathid, building_tuple[0], building_tuple[1], path_type,lookupNames[i]))
        
        # Iterate through each node, get its id from the hashmap,
        # then insert it into our list
        index = 1
        n = len(path)
        for i in range(0, n):
            node = path[i]
            nodeid = node_map[node]

            # Get edge cost if this isn't the final node
            if i < n - 1:
                next_node_id = node_map[path[i + 1]]
                cost = edge_dict[(nodeid, next_node_id)]
            else:
                cost = 0

            # Append this, then increment index
            pathmembersappend((pathid, nodeid, index, cost))
            index += 1

        # Increment pathID
        pathid += 1

    # Finally run an execute many to insert every value into the DB
    print("Inserting into DB...")
    c.execute("PRAGMA LOCKING_MODE = EXCLUSIVE")
    c.executemany("INSERT INTO ShortPathMembers VALUES (?, ?, ?, ?)", path_members)
    c.executemany("INSERT INTO ShortPaths VALUES (?,?,?,?,?)", path_headers)

    c.close()
    closeConn(conn)

    return path_headers,path_members


def addShortPaths(path_list, start_end_pairs, cost_type):
    """ Store a list of building to building paths to the database.

        Keyword Args:
        path_list -- a list of lists of (x,y,z) node tuples
        start_end_pairs -- list of tuples conatining the start building
                           and end building for each path of matching
                           index

    """
    # dropPaths(True)

    if cost_type == None:
        path_type = "Distance"
    else:
        path_type = cost_type

    s = time.time()

    # Get Hashmap
    node_map = getHashMap()
    print('Got node map')
    edge_dict = getEdgeDictionary(cost_type)
    print('got edge dict')
    # print('Got edges: ')
    # print(edge_dict)
    conn = openConn()
    c = conn.cursor()

    # Get the maximum path id
    c.execute(
        """SELECT pathID
                FROM ShortPaths
                ORDER BY ShortPaths.pathID DESC
                LIMIT 1"""
    )
    potential_id = c.fetchall()
    
    # Check if this fails since there may not be any paths in the DB
    pathid = potential_id[0][0] + 1 if len(potential_id) > 0 else 1

    # Cache these methods for faster appends.
    path_members = []  # Tuples (pathid, nodeid, index)
    path_headers = []  # tuples (pathid, building_A, building_B)

    pathmembersappend = path_members.append
    pathheadersappend = path_headers.append


    # Iterate through each path to precompute our insertion
    for i in range(0, len(path_list)):
        # Create a path header of (pathid, start_building, end_building)
        building_tuple = start_end_pairs[i]
        path = path_list[i]
        pathheadersappend((pathid, building_tuple[0], building_tuple[1], path_type,''))

        # Iterate through each node, get its id from the hashmap,
        # then insert it into our list
        index = 1
        n = len(path)
        for i in range(0, n):
            node = path[i]
            nodeid = node_map[node]

            # Get edge cost if this isn't the final node
            if i < n - 1:
                next_node_id = node_map[path[i + 1]]

                cost = edge_dict[(nodeid, next_node_id)]

            else:
                cost = 0

            # Append this, then increment index
            pathmembersappend((pathid, nodeid, index, cost))
            index += 1

        # Increment pathID
        pathid += 1

    # Finally run an execute many to insert every value into the DB
    print("Inserting into DB...")
    c.execute("PRAGMA LOCKING_MODE = EXCLUSIVE")
    c.executemany("INSERT INTO ShortPathMembers VALUES (?, ?, ?, ?)", path_members)
    c.executemany("INSERT INTO ShortPaths VALUES (?,?,?,?,?)", path_headers)

    c.close()
    closeConn(conn)

    return path_headers,path_members


def addArrayPaths(paths):
    """ Insert paths into the DB as lists of nodes """
    global pathname
    global dbname

    conn = openConn()
    c = conn.cursor()
    # Convert lists to strings
    # prepared_paths  = [(path[0], path[1], repr(path[2])[1:-1]) for path in paths]
    prepared_paths = arrayTest.storePaths(paths, filename=pathname)

    # Drop this table and recreate it

    c.execute("DROP TABLE IF EXISTS PicklePaths")
    c.execute(
        """CREATE TABLE IF NOT EXISTS PicklePaths(
            pathID INTEGER PRIMARY KEY AUTOINCREMENT,
            A VARCHAR,
            B VARCHAR,
            idx INTEGER,
            length INTEGER
            )"""
    )

    c.executemany("INSERT INTO PicklePaths VALUES(NULL,?,?,?,?)", prepared_paths)
    closeConn(conn)


def listPaths(paths):
    """ Insert paths into the DB as lists of nodes """

    # Convert lists to strings
    prepared_paths = [(path[0], path[1], repr(path[2])[1:-1]) for path in paths]

    # Drop this table
    conn = openConn()
    c = conn.cursor()
    c.execute("DROP TABLE IF EXISTS PicklePaths")
    c.execute(
        """CREATE TABLE IF NOT EXISTS PicklePaths(
            pathID INTEGER PRIMARY KEY AUTOINCREMENT,
            A VARCHAR,
            B VARCHAR,
            path BLOB
            )"""
    )

    # Convert each path into a string representation
    c = conn.cursor()
    c.executemany("INSERT INTO PicklePaths VALUES(NULL,?,?,?)", prepared_paths)
    closeConn(conn)


def getArrayPaths():
    """ Get listed path from the DB and return as a list of ints """

    conn = openConn()
    c = conn.cursor()

    global pathname

    c.execute("SELECT length FROM PicklePaths")
    lengths = [length[0] for length in c.fetchall()]
    paths = arrayTest.loadPaths(lengths, filename=pathname)

    return paths


def unstringPath(path):
    return [int(node) for node in path.split(",")]


def getListPaths():
    """ Get listed path from the DB and return as a list of ints """
    conn = openConn()
    c = conn.cursor()

    c.execute("SELECT path FROM PicklePaths")
    paths = c.fetchall()
    return [[int(node) for node in path[0].split(",")] for path in paths]


def quicklyAddPaths(path_members, path_headers):
    """ Store a list of building to building paths to the database.

        Keyword Args:
        path_list -- a list of lists of (x,y,z) node tuples
        start_end_pairs -- list of tuples conatining the start building
                           and end building for each path of matching
                           index

    """

    conn = openConn()
    c = conn.cursor()
    dropPaths(True)
    # Finally run an execute many to insert every value into the DB
    print ("Inserting into DB...")
    c.execute("PRAGMA LOCKING_MODE = EXCLUSIVE")
    c.executemany("INSERT INTO ShortPathMembers VALUES (?, ?, ?, ?)", path_members)
    c.executemany("INSERT INTO ShortPaths VALUES (?,?,?)", path_headers)
    print ("Creating Indexes...")

    # Create Indexes
    c.execute("CREATE INDEX i on ShortPathMembers(pathID)")
    c.execute("CREATE INDEX ShortExits on Shortpaths(A)")

    c.close()
    closeConn(conn)


def executeMultiQuery(query, targets, cursor):
    """ Safely execute an IN query
        query should be in form of SELECT ... WHERE item IN """

    # The unique problem here is that sql can only support where clauses with 999 question marks
    # so we need to iterate through the list 1000 elements at a time

    query += " (%s)"
    results = []
    for i in range(0, len(targets), 998):
        l = tuple(t for t in targets[i : i + 998])
        q = query % ",".join("?" for i in l)

        cursor.execute(q, l)
        results.append(cursor.fetchall())

    return results


def getShortPathsMultiID(ids):

    conn = openConn()
    c = conn.cursor

    c.execute(
        "SELECT pathID, nodeid, weight_to_next FROM ShortPathMembers WHERE pathID=?",
        (int(id),),
    )
    nodes = c.fetchall()
    closeConn(conn)

    r = [node[0] for node in nodes]
    # Return a list of costs if asked for
    if ret_costs:
        p = [node[1] for node in nodes]
        return r, p
    return r

def getPathByID(ids=[0], ret_costs=False):
    """ 
    Retrieve the shortest path for a given list of IDs
    If requesting a single path, pass an array of one element 
    """

    conn = openConn()
    c = conn.cursor()

    node_list = getHashMap(True)

    #if only one id is passed, parse the data and return 
    if len(ids) == 1:
        id = ids[0]
        # Get the entirety of the path members table
        c.execute("SELECT * FROM ShortPathMembers WHERE pathID = ?", (id,))
        pathData   = c.fetchall()
        nodeIDs    = [ x[1] for x in pathData]
        nodeIDXs   = [ x[2] for x in pathData]
        edgeCost   = [ x[3] for x in pathData]
        pathPoints = [ node_list[x] for x in nodeIDs ]

        closeConn(conn)

        return nodeIDs,nodeIDXs,edgeCost,pathPoints

    #TODO pass array of ids and get back nested paths 
    pathDataset = []
    for id in ids:
        # Get the entirety of the path members table
        c.execute("SELECT * FROM ShortPathMembers WHERE pathID = ?", (id,))
        pathRes   = c.fetchall()
        pathDataset.append((id,pathRes))
    closeConn(conn)

    #parse and store data in dictionary
    pathDataDict = {}
    for pathVals in pathDataset:
        id = pathVals[0]
        pathData = pathVals[1]
        nodeIDs    = [ x[1] for x in pathData]
        nodeIDXs   = [ x[2] for x in pathData]
        edgeCost   = [ x[3] for x in pathData]
        pathPoints = [ node_list[x] for x in nodeIDs ]
        #put data into dictionary of arrays 
        pathDataDict[id] = [nodeIDs,nodeIDXs,edgeCost,pathPoints ] 

    return pathDataDict

def getPathIDs():
    """ Retrieve the shortest path ids and lookup values
    """
    conn = openConn()
    c = conn.cursor()

    # Get the entirety of the path table
    c.execute("SELECT * FROM ShortPaths LIMIT 1000000")
    pathData = c.fetchall()
    closeConn(conn)
    return pathData

def getShortPaths(id=0, ret_costs=False, name="Distance"):
    """ Retrieve the shortest paths from every building to every
        building.

        KeywordArgs:
        id(Optional) -- the ID of the path to return. If not specified
                        return every path.

        ret_paths(Optional) -- if true, a secondary list of costs will
                               also be returned of form
                               [[node_to_node2,node2_to_node3, ...]
        name (Optional) -- if set: only return paths with the specified type
                           also be returned of form
                           [[node_to_node2,node2_to_node3, ...]]
    """

    conn = openConn()
    c = conn.cursor()

    if name:
        # Get the headers for every path involved
        c.execute("SELECT pathID from ShortPaths WHERE type = ?", (name,))

        res = c.fetchall()
        # if it's successful, then just run this again with the list of ids
        if len(res) >= 0:
            id = [r[0] for r in res]
        else:
            return []

    node_list = getHashMap(True)

    # If an ID is specified, only return the path belonging to that ID
    if (isinstance(id, int) or isinstance(id, float)) and id > 0:
        c.execute(
            "SELECT nodeid, weight_to_next FROM ShortPathMembers WHERE pathID=?",
            (int(id),),
        )
        nodes = c.fetchall()
        closeConn(conn)

        r = [node_list[node[0]] for node in nodes]
        # Return a list of costs if asked for
        if ret_costs:
            p = [node[1] for node in nodes]
            return r, p
        return r

    # If ID is a list return multiple
    elif isinstance(id, list):
        paths = []
        prices = []
        # For each path get all nodes listed as members
        for i in id:
            c.execute(
                "select nodeid, weight_to_next FROM ShortPathMembers WHERE pathID = ?",
                (i,),
            )
            path = []  # A list of nodes
            cost = []  # A list of costs
            for node in c.fetchall():
                path.append(node_list[node[0]])
                cost.append(node[1])
            paths.append(path)
            prices.append(cost)

        closeConn(conn)
        if ret_costs:
            return paths, prices
        return paths

    print ("Getting All!")
    # And if none of that works, return every path!
    numpaths = getStats()["shortpaths"]

    # Add a list for every path to our list of lists
    # path_lists = dict.fromkeys(xrange(0,numpaths))
    path_lists = []
    for i in range(0, numpaths + 1):
        path_lists.append([])

    start = time.time()
    # Get the entirety of the path members table
    c.execute("SELECT * FROM ShortPathMembers LIMIT 1000000")
    paths = c.fetchall()
    # print "Fetch Time", time.time() - start

    # Sort through each
    for member in paths:
        pathid = member[0]
        nodeid = member[1]
        index = member[2]

        path_lists[pathid].insert(index, nodeid)
    closeConn(conn)
    return path_lists


def getExitPaths(start_building):
    """ Get all paths from the specified building and return as a list
        of lists.  """

    conn = openConn()
    c = conn.cursor()

    print (" Getting Exit nodes for ", start_building)

    # Get the ids for each path with this building as A
    c.execute("SELECT pathID FROM ShortPaths WHERE A = ?", (start_building,))

    pathids = [result[0] for result in c.fetchall()]
    closeConn(conn)

    return getShortPaths(pathids, True)


def addPaths(paths, ranks):
    """ Add paths to the database as format [(x,y,z), (x,y,z)]"""

    print ("Adding paths to the Db ")
    # Construct hashmap of nodes
    node_map = getHashMap()
    conn = openConn()
    c = conn.cursor()

    input_tuples = []

    # Convert each path into a list of tuples
    input_tuples = itertools.chain(
        *[
            [(i, node_map[paths[i][k]], k) for k in range(0, len(paths[i]))]
            for i in range(0, len(paths))
        ]
    )

    # Insert into the DB
    c.executemany("INSERT INTO Path_Members VALUES(?,?,?)", input_tuples)
    c.execute("CREATE INDEX pathindex ON Path_Members(pathID)")
    closeConn(conn)


def dropPaths(short=False):
    """ Clear all paths out of the database, but preserves other data"""
    conn = openConn()
    c = conn.cursor()

    if short:
        c.execute("DROP TABLE ShortPathMembers")
        c.execute("DROP TABLE ShortPaths")
        c.execute("DROP TABLE PicklePaths")
        closeConn(conn)
        createTables()
        return

    c.execute("DROP INDEX IF EXISTS pathindex")
    c.execute("DELETE FROM Paths")
    c.execute("DELETE FROM Path_Members")
    closeConn(conn)


def deletePaths(pathID):
    """ Delete the path at the given ID from the Database """
    conn = openConn()
    c = conn.cursor()

    c.executemany("DELETE FROM Paths WHERE pathID = (?)", pathID)
    c.executemany("DELETE FROM Path_Members WHERE pathID = (?)", pathID)

    closeConn(conn)


def getStats():
    """ Return a dictionary of assorted database stats.

        Valid keys are nodes, paths, links, shortpaths """
    stats = {}
    stats["name"] = dbname

    # Open Connection
    conn = openConn()
    c = conn.cursor()

    # Get number of nodes
    c.execute("SELECT COUNT(*) FROM Nodes")
    stats["nodes"] = c.fetchone()[0]

    # Get number of links
    c.execute("SELECT COUNT(*) FROM Edges")
    stats["links"] = c.fetchone()[0]

    # Get number of paths
    c.execute("SELECT COUNT(*) FROM Paths")
    stats["paths"] = c.fetchone()[0]

    # Get number of shortest paths
    c.execute("SELECT COUNT(*) FROM ShortPaths")
    stats["shortpaths"] = c.fetchone()[0]
    closeConn(conn)
    return stats


def getPaths(pathID=0):
    """ Get all jogging paths from the DB and return them.

        If pathID is specified, only return the path matching that ID"""

    # Construct a hashmap for all nodes
    node_map = getHashMap(True)

    conn = openConn()
    c = conn.cursor()

    # Check to see if the user specified a path to get
    if pathID > 0:
        c.execute(
            "SELECT * FROM Path_Members WHERE pathID = ? ORDER BY NodeIndex", (pathID,)
        )

    # If not, give them everything
    else:
        c.execute("SELECT * FROM Path_Members ORDER BY NodeIndex")
    members = c.fetchall()
    closeConn(conn)

    # Iterate through each member and assign it to the right path
    paths = {}
    for node in members:
        if node[0] in paths:
            # This list exists, just append
            paths[node[0]].append(node_map[node[1]])
        else:
            paths[node[0]] = [node_map[node[1]]]

    return paths.values()


def addBuildings(buildings):
    """ Adds a set of buildings to the DB straight from the output of the
        building classifier """
    conn = openConn()
    c = conn.cursor()
    built = []

    # Precompute our query. Round all numeric values
    for building, stats in buildings.items():
        bclass = stats[0]
        volume = round(stats[1], 2)
        area = round(stats[2], 2)
        height = round(stats[3], 2)
        built.append((str(building), bclass, volume, area, height))

    # Insert into DB and create an index on building id
    c.executemany("INSERT INTO Buildings VALUES (?,?,?,?,?, null)", built)
    c.execute("CREATE INDEX BuildingIndex ON Buildings(BuildID)")
    closeConn(conn)


def getBuildings(buildingID="all"):
    """ Returns building stats as a dictionary of the form:
        {buildingID: [class, volume, squarefootage, height, exitnode]}"""
    conn = openConn()
    c = conn.cursor()

    # We have input, so only return the specified building
    if buildingID != "all":
        c.execute("SELECT * FROM Buildings WHERE buildID = ?", (buildingID,))
    # No input, so give all buildings
    else:
        c.execute("SELECT * FROM Buildings")

    buildings = c.fetchall()
    closeConn(conn)

    # Construct output dictionary
    building_dict = {}
    for building in buildings:
        building_id = building[0]
        building_class = building[1]
        building_volume = building[2]
        building_squarefootage = building[3]
        building_height = building[4]
        building_exit = building[5]
        building_dict[building_id] = [
            building_class,
            building_volume,
            building_squarefootage,
            building_height,
            building_exit,
        ]

    return building_dict


def setExitNode(nodes):
    """Set the exit node for a list of (exit node, building) pairs."""

    conn = openConn()
    c = conn.cursor()

    # TODO: Replace this with a list comp and executemany
    for node in nodes:
        buildID = node[0]
        nodeid = node[1]

        inData = (nodeid, buildID)

        c.execute("UPDATE Buildings SET exitNode=? WHERE buildid = ?", inData)

    closeConn(conn)


def getHashMap(reverse=False):
    """ Generate a dictionary of nodes (x,y,z): ID.

        If reverse is specified, the returned dictionary will be of
        the form ID:(x,y,z)
    """

    conn = openConn()
    c = conn.cursor()

    # Get every Node in DB
    c.execute("SELECT nodeID, nodeX, nodeY, nodeZ FROM Nodes ORDER BY nodeID")
    db_nodes = c.fetchall()

    # If reverse is specified, find id from (x,y,z)
    if not reverse:
        gen = {(node[1], node[2], node[3]): node[0] for node in db_nodes}
    else:
        gen = {node[0]: (node[1], node[2], node[3]) for node in db_nodes}

    closeConn(conn)
    return gen


def getExitNodes():
    """ Get a list of tuples (buildID, exitnode) from the DB and return."""
    conn = openConn()
    c = conn.cursor()

    c.execute("SELECT buildID, exitnode FROM Buildings")
    exitNodes = c.fetchall()
    closeConn(conn)
    return exitNodes

def getLinkMap(reverse=False):
    """
    returns dictionary of {(edge1,edge2): linkid}
    reverse flag gives key:value as {linkID:edge}
    """

    conn = openConn()
    c = conn.cursor()

    c.execute( "SELECT edgeID,parentID,childID FROM Edges" )

    linkIDs = c.fetchall()
    closeConn(conn)

    if reverse == True: 
        linkIDsDict = { x[0]:(x[1],x[2]) for x in linkIDs  }
        return linkIDsDict

    linkIDsDict = { (x[1],x[2]):x[0] for x in linkIDs  }

    return linkIDsDict


def getLinkFromNodes(node1,node2,edges=None):
    """
    Get the link ID between two nodes
    if edges is passed as a list of tuples, it will be used to do a single
    query on the DB and reconstruct in python as a dictionary map

    This will be slow if called in a loop, instead call getLinkMap() 
    and use it to construct the data yourself. 
    """

    conn = openConn()
    c = conn.cursor()

    start_time = time.time() 

    if edges!=None:
        c.execute( "SELECT edgeID,parentID,childID FROM Edges" )

        linkIDs = c.fetchall()
        closeConn(conn)

        # edgeIDs = [ x[0] for x in linkIDs ]
        # parentIDs = [ x[1] for x in linkIDs ]
        # childIDs = [ x[2] for x in linkIDs ]

        linkIDsDict = { (x[1],x[2]):x[0] for x in linkIDs  }

        return linkIDsDict

    edge = (node1,node2)

    c.execute(
        "SELECT edgeID FROM Edges WHERE parentID = (?) and childID = (?)", edge
    )

    #value is returned as a 2d array
    #there should only be one linkID value for an edge though
    linkID = c.fetchall()
    closeConn(conn)

    return linkID

def getEdgeCosts(paths,costType=None):
    """
    Get the edge costs for a given list of paths 
    each path is an array of edges
    """
    conn = openConn()
    c = conn.cursor()

    c.execute("SELECT linkID,paramValue FROM Link_Param WHERE paramName=?",(costType,))
    linkData = c.fetchall()
    closeConn(conn)

    #make linkData a dictionary of { linkID: cost }
    costDict = { x[0]:x[1] for x in linkData  }
    #get a mapping of {linkID:edge}
    linkMap = getLinkMap()

    pathCosts = []
    for path in paths: 
        #convert the edge of the path to a linkID for getting the cost
        costs = { x:costDict[linkMap[x]] for x in path }
        pathCosts.append(costs)

    return pathCosts

def getLinkDict(costKey=None):
    """
    Get all link parameters as a dictionary 
    pass an optional costKey to filter link parameters
    """
    start_time = time.time()

    conn = openConn()
    c = conn.cursor()

    rows=c.execute("SELECT linkID,paramName,paramValue FROM Link_Param")
    linkData = rows.fetchall()
    closeConn(conn)

    linkParamDict =  defaultdict(list)
    #if the glareLinks flag is set to true, filter links by glare
    if costKey != None:
        for link in linkData:
            if costKey not in link[1]: continue 
            date = link[1].split('_')[1]
            linkParamDict[link[0]].append((date,link[2]))

        return linkParamDict 

    for link in linkData:
        linkParamDict[link[0]].append((link[1],link[2]))

    return linkParamDict 

def getPathLinkParameters(linkIDs):
    """ 
    Get link paramaters as array 
    use is to save time by having an existing connection open compared to calling
    getLinkParameters in a loop 
    """
    conn = openConn()
    c = conn.cursor()

    pathParams = []
    for linkid in linkIDs:
        sql="SELECT paramName,paramValue,linkID FROM Link_Param WHERE linkID IN ({seq})".format(seq=','.join(['?']*len(linkid)))
        rows=c.execute(sql, linkid)
        data = rows.fetchall()
        pathParams.append(data) 
    closeConn(conn)
    return pathParams


def getLinkParameters(node1=None, node2=None, linkid=None,paramName=None):
    """ Get link parameters for the link connecting (node1, node2).

        If param_name is specified, only return the value of the given
        parameter.
    """
    conn = openConn()
    c = conn.cursor()

    #if no linkid was given, get it using the nodes
    if linkid == None: linkid = getLinkFromNodes(node1,node2)[0][0]
    
    if type(linkid) != type(1):
        # c.execute(
        #     "SELECT paramName,paramValue,linkID FROM Link_Param WHERE linkID IN ({0})".format(
        #         ', '.join('?' for _ in linkid) ), linkid
        # )
        # ret = c.fetchall()

        ## Straightforward method 
        sql="select paramName,paramValue,linkID FROM Link_Param WHERE linkID IN ({seq})".format(seq=','.join(['?']*len(linkid)))
        rows=c.execute(sql, linkid)
        data = rows.fetchall()
        closeConn(conn)
        return data

    #if no paramater name is given, get it 
    if paramName == None:
        c.execute("SELECT paramName, paramValue FROM Link_Param WHERE linkID = (?)", (linkid,) )
        ret = c.fetchall()
        closeConn(conn)
        return ret

    c.execute("SELECT linkID, paramValue FROM Link_Param WHERE paramName = (?)", (paramName,) )
    ret = c.fetchall()
    closeConn(conn)
    return ret


def getNodeParameters(paramName):
    """ Return the value of all nodes that have the specified parameter.

        Returns as a list of tuples for (nodeid, param_value)
    """
    conn = openConn()
    c = conn.cursor()
    c.execute("SELECT nodeid, paramValue FROM Node_param WHERE paramName = (?)", (paramName,) )

    ret = c.fetchall()
    closeConn(conn)
    return ret


def getBuildingParameter(buildID, paramName=None):
    """ Return the value of all buildings that have the specified parameter set

        Returns as a list of tuples for (building_id, param_value)
    """
    conn = openConn()
    c = conn.cursor()

    c.execute(
        "SELECT paramValue FROM Building_Param WHERE paramName = (?)", (paramName,)
    )

    out = c.fetchall()
    closeConn(conn)

    return out


def dropParameter(name):
    """Remove all instances of param name from the database"""

    conn = openConn()
    c = conn.cursor()
    c.execute("DELETE FROM Node_Param WHERE paramName = (?)", (name,))

    closeConn(conn)

def setNodeParameters(tuples, purge=True):
    """ Input a list of tuples of (nodeid, paramname, paramvalue)
        to the DB.

        Setting purge to True will purge the DB of each parameter name specified
        in one of the tuples.
    """
    # First, let us create a list of tuples for rows to delete
    conn = openConn()
    c = conn.cursor()

    # Check if the user wants to delete all instances of this param, or just
    # this one
    if not purge:  # Only delete rows we intend to replace
        rows_to_replace = ((t[0], t[1]) for t in tuples)
        c.executemany(
            "DELETE FROM Node_Param WHERE nodeID = (?) AND paramName = (?)",
            rows_to_replace,
        )

    else:  # Scorched earth
        dropped_params = []

        # Iterate through each tuple to get names
        for t in tuples:
            param = t[1]

            # Check to see if we've dropped this one yet, if not, drop it and
            # record it
            if param not in dropped_params:
                dropParameter(param)
                dropped_params.append(param)

    # Readd them with new values
    c.executemany("INSERT INTO Node_Param VALUES (?,?,?)", tuples)
    closeConn(conn)


def setLinkParameters(inputs):
    """ Set link parameters for a list of links of the form
        (node1, node2, paramName, paramValue)
    """
    conn = openConn()
    c = conn.cursor()

    # First we must acquire the link ID from node1,node2
    link_dict = getEdgeDictionary()
    # Generate list of input tuples

    input_tuples = (
        (link_dict[(link[0], link[1])], link[2], link[3]) for link in inputs
    )
    # Generate list of rows to replace
    rows_to_replace = ((tup[0], tup[1]) for tup in input_tuples)

    # Delete parameter assignments if they already exist
    c.executemany(
        "DELETE FROM Link_Param WHERE linkID = ? AND paramName = ?", rows_to_replace
    )

    # Insert our rows
    c.executemany("INSERT INTO Link_Param VALUES (?,?,?)", input_tuples)

    closeConn(conn)


def setBuildingParameter(parameter_pairs):
    """ Sets parameters for a list of tuples of the form
        (buildingid, param_name, param_value)
    """
    conn = openConn()
    c = conn.cursor()

    # Get a list of rows to delete
    rows_to_replace = ((tup[0], tup[1]) for tup in parameter_pairs)

    # Delete existing pairings
    c.executemany(
        "DELETE FROM Building_Param WHERE BuildingID = ? AND paramName = ?",
        rows_to_replace,
    )

    # Insert New Pairings
    c.executemany("INSERT INTO Building_Param VALUES (?,?,?)", parameter_pairs)
    closeConn(conn)


def setEdgeCosts(inputs, name):
    """ Set link parameters for a list of links of the form
        (node1, node2, value) and parametername
    """

    global dbname
    dbName = dbname[4:-3]
    link_dict = getEdgeDictionary("ID")

    conn = openConn()
    c = conn.cursor()

    # Generate list of input tuples of form (nodeid, param_name, param_value)
    db_entries = [(link_dict[(link[0], link[1])], name, link[2]) for link in inputs]

    # Generate list of rows to replace from input dictionary
    rows_to_replace = [(tup[0], tup[1]) for tup in db_entries]

    # Delete parameter assignments if they already exist
    c.executemany(
        "DELETE FROM Link_Param WHERE linkID = ? AND paramName = ?", rows_to_replace
    )
    # Insert rows
    c.executemany("INSERT INTO Link_Param VALUES (?,?,?)", db_entries)
    closeConn(conn)

    # Delete Associated Matrixes
    directory = "graphs"
    FWdir = directory + "\\" + dbName + name
    FWdirNPY = FWdir + ".npy"
    predDir = directory + "\\" + dbName + name + "_pred"
    predDirNPY = predDir + ".npy"
    graph_path = directory + "\\" + dbName + name + "_sparse.npz"

    try:
        os.remove(graph_path)
        os.remove(predDirNPY)
        os.remove(FWdirNPY)
    except Exception as e:
        # print("NO DELETE")
        # print graph_path
        # print predDirNPY
        # print FWdirNPY
        pass


def closeConn(conn):
    """ Check to see if optimizations must be made, then
        close the given connection"""
    conn.execute("PRAGMA optimize")
    conn.commit()
    conn.close()

def addToQueue(AtoB):
    """ Add a list of A to B tuples to the queue"""

    conn = openConn()
    c = conn.cursor()

    c.executemany("INSERT OR IGNORE INTO PATH_Queue VALUES(?,?, ?)", AtoB)

    closeConn(conn)


def getQueue(parameter=None):
    """ Get a list of A to B pairs from the queue. Takes an optional string
        'parameter' to only retrieve pairs with that tag."""
    conn = openConn()
    c = conn.cursor()

    if parameter is None:
        c.execute("SELECT * FROM Path_Queue")
    else:
        c.execute("SELECT * FROM Path_Queue WHERE type = ?", (parameter,))
    pairs = c.fetchall()
    closeConn(conn)
    return pairs


def clearQueue():
    """ Remove all tuples from the queue"""
    conn = openConn()
    c = conn.cursor()

    pairs = c.execute("DROP TABLE IF EXISTS Path_Queue")
    c.execute(
        """ CREATE TABLE IF NOT EXISTS Path_Queue(
                A int,
                B int,
                type Text,
                UNIQUE(A,B,type))
        """
    )
    closeConn(conn)


def generatePermutations(l):
    return itertools.permutations(l, 2)


def getClosestNode(nodes):
    """ Returns a list of node ids for nodes closest to existing nodes """
    ids = []
    db_nodes = getNodes()
    for node in nodes:
        distances = [
            sqrt(
                pow((node[0] - db_node[0]), 2)
                + pow((node[1] - db_node[1]), 2)
                + pow((node[2] - db_node[2]), 2)
            )
            for db_node in db_nodes
        ]
        ids.append(distances.index(min(distances)) + 1)

    return ids




def clearFuncArgs(paramKey=None):
    """
    Delete the paramKey user parameters from table, 
    if no paramKey is given, delete all the parameters
    """
    conn = openConn()
    c = conn.cursor()

    if paramKey == None:
        c.execute("DELETE FROM UserParams")
        closeConn(conn)
        return

    c.execute("DELETE FROM UserParams WHERE paramKey = ? ",(paramKey,)) 
    closeConn(conn)
    return 

def addFuncArgs(paramKey,funcArgs,paramVer=None):
    """
    TODO: Add column for version so multiple user settings can be added with same keyword

    Allows user to store arguments for a function
    Can be used to store environment settings as well

    takes in a keyword and dictionary of arguments 
    """
    conn = openConn()
    c = conn.cursor()

    json_string = json.dumps( funcArgs )

    if paramVer == None: paramVer = 1

    c.execute("SELECT * from UserParams WHERE paramKey = ? AND paramVer = ?",(paramKey,paramVer))
    existing = c.fetchall()

    #if this param key and version doesnt exist, add it, otherwise, update
    if len(existing) == 0: 
        c.execute("INSERT INTO UserParams (paramKey,paramVer,args) VALUES(?,?,?)",
                    (paramKey,paramVer,json_string)  )

    else: 
        c.execute("UPDATE UserParams SET args = ? WHERE paramKey = ? AND paramVer = ?",
                    (json_string,paramKey,paramVer)  )

    closeConn(conn)

    return 

def getFuncArgs(paramKey,latest=False):
    """
    Returns a dictionary of the parameters associated in the DB with the keyword
    if more than one param version is found, a dictionary with all versions is returned 
    if the latest flag is true, it will only return the latest version 
    """
    sqlite3.register_converter("json", json.loads)

    conn = openConn()
    c = conn.cursor()

    c.execute("SELECT paramVer,args from UserParams WHERE paramKey = ?",(paramKey,))
    funcArgs = c.fetchall()
    closeConn(conn)

    #if no arguments found, print message and return None
    if len(funcArgs) == 0:
        print('No Arguments Found')
        return None

    #if only one argument found, return it
    if len(funcArgs) == 1: 
        funcArgs = funcArgs[0][1] 
        return funcArgs

    #if the latest argument is asked for, return just that one 
    if latest == True:
        funcArgs = funcArgs[-1][1]
        return funcArgs

    funcDict = { x[0]:x[1] for x in funcArgs  }

    return funcDict
