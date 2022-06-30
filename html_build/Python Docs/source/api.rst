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


.. currentmodule:: humanfactorspy
  
.. automodule:: humanfactorspy.common_native_functions
    :members:
    

.. rubric:: Exceptions

.. automodule:: humanfactorspy.Exceptions
    :members:
    

.. rubric:: Numpy Interface 

.. automodule:: humanfactorspy.native_numpy_like
    :members:
    

Graph Generator
---------------

.. automodule:: humanfactorspy.graphgenerator.graph_generator_native_functions
    :members:


Geometry
--------

.. automodule:: humanfactorspy.geometry.meshinfo_native_functions
    :members:


Path Finding
------------

.. automodule:: humanfactorspy.pathfinding.pathfinder_native_functions
    :members:


Raytracer
---------

.. automodule:: humanfactorspy.raytracer.raytracer_native_functions
    :members:


Spatial Structures
------------------

.. automodule:: humanfactorspy.spatialstructures.spatial_structures_native_functions
    :members:


View Analysis
-------------

.. automodule:: humanfactorspy.viewanalysis.viewanalysis_native_functions
    :members:


Visibility Graph
----------------

.. automodule:: humanfactorspy.visibilitygraph.visibility_graph_native_functions
    :members: