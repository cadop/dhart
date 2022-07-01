==========================
Internal API Documentation
==========================

API Overview
============

The API differs from the general code reference as it is only internal methods used for 
communicating between the core .dll and the python interface.  

Nothing here should be called by a user. This is only for developer reference. 


Native Functions to C Interface
===============================

Main Module
-----------


.. currentmodule:: dhart
  
.. automodule:: dhart.common_native_functions
    :members:
    

.. rubric:: Exceptions

.. automodule:: dhart.Exceptions
    :members:
    

.. rubric:: Numpy Interface 

.. automodule:: dhart.native_numpy_like
    :members:
    

Graph Generator
---------------

.. automodule:: dhart.graphgenerator.graph_generator_native_functions
    :members:


Geometry
--------

.. automodule:: dhart.geometry.meshinfo_native_functions
    :members:


Path Finding
------------

.. automodule:: dhart.pathfinding.pathfinder_native_functions
    :members:


Raytracer
---------

.. automodule:: dhart.raytracer.raytracer_native_functions
    :members:


Spatial Structures
------------------

.. automodule:: dhart.spatialstructures.spatial_structures_native_functions
    :members:


View Analysis
-------------

.. automodule:: dhart.viewanalysis.viewanalysis_native_functions
    :members:


Visibility Graph
----------------

.. automodule:: dhart.visibilitygraph.visibility_graph_native_functions
    :members: