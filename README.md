
# Overview

Welcome to DHART:a C++ package with interfaces to Python, C, and C# for  Design, Humans, Analysis, and RoboTics.

There are a few components to the package, but the main focus is in providing fast ray-casting interfaces for python and C# for a variety of analysis and evaluation techniques.  Generally, you should be able to build a grid-like structure of the floors for a 3D model, calculate shortest paths by distance and custom metrics, and use these tools inside of a variety of programs such as Unity, Rhino, Revit, and more. 

We have extensive documentation on the API, and welcome new contributions and bug fixes. Please make sure to take a look at the contributing guide.  

Features
--------

- Python, C, C#, C++ interface
- Extract connected grid-based graphs from 3d models
- Calculate shortest path based on Distance, Energy, Visibility, and others
- Graph parsing traverses stairs and slopes
- Easily generate visibility graphs and analyze locations of environment
- Calculate view scores, percentage of view, and points of interest
- Uses Embree raytracer for fast raycasting
- Demos and instructions for integrating with Rhino 3D and Unity


Credits
-------


This repository contains work that was supported in part by the U.S.
Army Combat Capabilities Development Command (CCDC) Armaments
Center and the U.S. Army ManTech Office under Contract Delivery
Order W15QKN19F0002 - Advanced Development of Asset Protection
Technologies (ADAPT).

If you find this repo useful, please cite using the following bibtex
```
@article{schwartz2021human,
  title={Human centric accessibility graph for environment analysis},
  author={Schwartz, Mathew},
  journal={Automation in Construction},
  volume={127},
  pages={103557},
  year={2021},
  publisher={Elsevier}
}
```

There were dozens of contributors to this project over the years.  It has been led by Mathew Schwartz (NJIT) with a large part of the development by Drew Balletto. 


Example Usage
-------------

- Python docs: https://cadop.github.io/dhart/Python%20Docs/build/html/index.html
- C++ and C Interface docs: https://cadop.github.io/dhart/C++/html/index.html
- C# docs: https://cadop.github.io/dhart/C%23%20Public%20Docs/html/index.html

Once the python package is installed, the basic setup for loading a model (e.g. obj), setting its rotation (if its not default z up), and creating a BVH (the accelerated structure of the mesh) is done by:

```
from dhart.geometry import LoadOBJ, CommonRotations
from dhart.raytracer import (EmbreeBVH,Intersect,
                                        IntersectForPoint,
                                        IntersectOccluded)
import dhart

# Get model path
obj_path = dhart.get_sample_model('plane.obj')
# Load mesh
loaded_obj = LoadOBJ(obj_path, rotation=CommonRotations.Yup_to_Zup)
# Create BVH
bvh = EmbreeBVH(loaded_obj)
```

After this, different methods for casting a ray can be used:

```
# Define point to start ray
p1 = (0, 0, 2)
# Define direction to cast ray
dir = (0, 0, -1)

# Cast a ray for the hitpoint
hit_point = IntersectForPoint(bvh, p1, dir, -1)
print(f"Hit point: {hit_point}")

# Cast a ray for distance/meshid
distance, mesh_id = Intersect(bvh, p1, dir, -1)
print(f"distance is {distance}, meshid is {mesh_id}")

# See if it occludes
does_occlude = IntersectOccluded(bvh, p1, (0, 0, -1), 9999)
print(f"Does the ray connect? {does_occlude}")
```

which would output

```
Hit point: (0.0, 0.0, 0.0)
distance is 2.0, meshid is 0
Does the ray connect? True
```

Getting started
===============


Installing
----------

- Requires Windows 10


We supply dll's to try and make the installation and linking process as easy as possible. 

Depending on your goal, you may want to download the Python or the C# release. Each one has a different process. 

When visiting the documentation page, click on Python to follow installation guide for running in a python environment or as a Python Grasshopper node for Rhino.  Click on the C# documentation for a guide on how to use the project with Unity. 


Building from Source
--------------------

### Prelim

This guide will describe how to retrieve the contents of this repository,
and build from source.

Please note: DHART is currently designed to run on a PC running the Windows 10
operating system.
Also note: This guide is a 'work-in-progress' and subject to change.

Git for Windows:
If you do not already have Git for Windows on your machine,
please download and install Git for Windows at https://gitforwindows.org/
Microsoft Visual Studio 2019:
https://visualstudio.microsoft.com/downloads/
You may use the Community edition.
CMake (Windows win64-x64)
https://cmake.org/download/


Once the software above is installed on your machine,
you may continue with the following:

0. Open Git Bash.
Git Bash (MINGW64) begins at ~, your home directory.
You may remain here, or navigate to a directory of your choice.
In the next step, you will clone the Analysis repository.

1. Type git clone git@github.com/cadop/dhart.git at the prompt
and hit ENTER. The Analysis repository will then be cloned to your local machine.


### Using CMAKE Commands

Currently we directly call the configuration arguments when using cmake. 

Python Debug

1. `cmake ./src/ -G"Visual Studio 16 2019" -DCMAKE_GENERATOR_PLATFORM="x64"  -DCMAKE_CONFIGURATION_TYPES="Debug" -DCMAKE_INSTALL_PREFIX=".\..\build\Python" -DHumanFactors_Config="All" -DHumanFactors_EnableTests="False" -DHumanFactors_EnablePython="True" -DHumanFactors_EnableCSharp="False" -DINSTALL_GTEST="False"  ".\" 2>&1` 

1. `cmake --build . --config Debug`

Python Release

1. `cmake ./src/  -G"Visual Studio 16 2019"  -DCMAKE_GENERATOR_PLATFORM="x64"   -DCMAKE_INSTALL_PREFIX=".\..\build\Python" -DHumanFactors_Config="All" -DHumanFactors_EnableTests="False" -DCMAKE_CONFIGURATION_TYPES="Release" -DHumanFactors_EnablePython="True" -DHumanFactors_EnableCSharp="False" -DINSTALL_GTEST="False"  ".\" 2>&1`

1. `cmake --build . --config Release`

1. `cmake --install .`

1. cd to build/Python and run `pip install .`

### Using Visual Studio



1. Open Microsoft Visual Studio 2019. After seeing the splash/welcome screen, please click the button that reads Open a local folder.


1. When the Browse window appears, navigate to the folder
(the repository that you had cloned) on your local machine.
Then, navigate to `src/`. Click the Select folder button to confirm.

1. You may see a banner that asks to generate or configure the cmake project.  Click generate. 

1. If the Solution Explorer view is not already open, you may open by
navigating to View > Solution Explorer, or alternatively, you can also use the Ctrl + Alt + L shortcut to reveal Solution Explorer. Here, you can examine the sources imported by Visual Studio.

1. We are now ready to build Analysis.
CMake is used to aid in the compilation process.
There are a few provided configuration files. Specifically, debug and release, as well as C# and Python specific ones.  Select the one you are interested in and navigate to Build > Build All. 

1. Once it has successfully built, you need to install.  Going back to the same build menu dropdown, click on install.  This will save the files to a build directory. 


You can find the build files in .\dhart\src\out\build\x64-Debug
