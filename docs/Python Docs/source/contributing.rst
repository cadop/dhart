.. _contr-test:

========================
Contributing and Testing
========================


The process assumes the repo has been cloned and was built using visual studio (with cmake), then using the install. This generates a 
build folder with the python package in it. The following instructions rely on this build folder (not the src folder).  

Check dependencies
------------------

Currently tested on ``Python 3.8+``.

For the package: ``numpy>=1.18``, ``scipy>=1.4.1``

For building documentation: ``sphinx>=3.0.3`` (latest version uses ``sphinx>=7``), ``scipy`` html_theme, ``matplotlib``.

All packages should be automatically installed if using ``pip install dhart[dev]`` (or ``pip install .[dev]`` if cloning the repo).


Building Documentation
----------------------

Documentation is built using sphinx with the scipy html_theme in the `docs` folder. 


``make clean``

``make html``

For windows, you may have to use ``.\make.bat html``.

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
