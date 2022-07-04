# DHART

# Table of contents
- Pre-requisites
- Building with Microsoft Visual Studio 2019
- Building with CMake's standalone client (cmake-gui)
- Documentation

- See the [Contributing Guide](docs/Contributing.md) for information on style guides.
- See the [Documentation Guide](docs/Documentation.md) for information on documenting and building documentation.


This guide will describe how to retrieve the contents of this repository,
and build from source.

<h2>Please note:
Dhart is currently designed to run on a PC<br>
running the Windows 10 operating system.</h2>
Also note: This guide is a 'work-in-progress' and subject to change.

<h2>Pre-requisites</h2>

<b>Git for Windows:</b><br>
If you do not already have Git for Windows on your machine,<br>
please download and install Git for Windows at https://gitforwindows.org/

<b>Microsoft Visual Studio 2019:</b><br>
https://visualstudio.microsoft.com/downloads/<br>
You may use the Community edition.

<b>CMake (Windows win64-x64) Version 3.8 or higher </b><br>
https://cmake.org/download/

Once the software above is installed on your machine,
you may continue with the following:

<h3>0. Using Git Bash</h3>
Open <b>Git Bash</b>.
Git Bash (MINGW64) begins at <code>~</code>, your home directory.<br>
You may remain here, or navigate to a directory of your choice.<br>
In the next step, you will clone the <b>dhart</b> repository.

<h3>1. Cloning the repository</h3>
Type <code><b>git clone git@github.com/cadop/dhart.git</b></code> at the prompt<br>
and hit ENTER. The repository will then be cloned to your local machine.

Now that a local copy of the <b>dhart</b> repository is on your machine,<br>
you may build the project with either:
- [Microsoft Visual Studio 2019](docs/BuildVS.md)
- [cmake-gui (CMake's standalone client)](docs/BuildCMake.md)


For a guide on how to obtain the output DLLS from this project after it has been generated and built, look at the guide on [the Cmake Installation Step](@ref InstallGuide)

# Documentation
Documentation for <b>dhart</b> can be found in ./dhart/docs/

The codebase is comprised of sources from:
<br>
- C++ (see ./dhart/docs/C++)
- C#  (see ./dhart/docs/C#)
- Python (see ./dhart/docs/Python Docs)

# C++
The C++ portion of the codebase uses a subset of the <b>Boost</b> libraries,<br>
and the sources are included with this repository.

Because the Boost sources are included with dhart,<br>
(in <code>dhart/src/external/boost</code>),<br>
you do <b>not</b> need to download or install Boost to build the repository.

You may read more about Boost at their website: https://www.boost.org/

TBA: C# details<br>
TBA: Python details<br>
