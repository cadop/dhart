============
Installation
============

Check dependencies
------------------
Currently tested on ``Python 3.8+``.

For the package: ``numpy>=1.18``, ``scipy>=1.4.1``

For building documentation: ``sphinx>=3.0.3``, ``scipy`` html_theme

Setup Human Factors
-------------------

``pip install .``

Building Documentation
----------------------

Documentation is built using sphinx with the scipy html_theme in the `docs` folder. 

``make clean``

``make html``


Testing
--------

Docstring examples should be compliant with ``doctest``. 
Navigate to the `docs` folder and run:

``make doctest``


You can also test with ``pytest`` by navigating to the `build\Python` directory and first run:

``pytest``

then run 

``pytest --doctest-modules --ignore-glob="*/Rhino*.py"``

to test all files except for the ones requiring Rhino.