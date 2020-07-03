"""
Store relationhships between points in space
=====================================================================


The graph is a datastructure that contains a series of nodes
and connections between them. Nodes can be created as 3D points
in space or just a set of IDs.

""" 


from .graph import Graph
from .node import NodeStruct, NodeList
from .edge import Edge