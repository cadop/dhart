# Analysis

Analysis of the Environment for ADAPT

# Table of contents
- Getting started
- Pre-requisites
- Documentation

# Getting started

This guide will describe how to retrieve the contents of this repository,
and build Analysis from source.

<h2>Please note:
Analysis is currently designed to run on a PC<br>
running the Windows 10 operating system.</h2>
Also note: This guide is a 'work-in-progress' and subject to change.

Pre-requisites:

<b>Git for Windows:</b><br>
If you do not already have Git for Windows on your machine,<br>
please download and install Git for Windows at https://gitforwindows.org/

<b>Microsoft Visual Studio 2019:</b><br>
https://visualstudio.microsoft.com/downloads/<br>
You may use the Community edition.

<b>CMake (Windows win64-x64)</b><br>
https://cmake.org/download/

Once the software above is installed on your machine,
you may continue with the following:

<b>0.</b>
Open <b>Git Bash</b>.
Git Bash (MINGW64) begins at <code>~</code>, your home directory.<br>
You may remain here, or navigate to a directory of your choice.<br>
In the next step, you will clone the <b>Analysis</b> repository.

<b>1.</b><br>
Type <code>git clone git@git.njit.edu:ADAPT/Analysis.git</code> at the prompt<br>
and hit ENTER. The Analysis repository will then be cloned to your local machine.

<b>2.</b><br>
Open <b>Microsoft Visual Studio 2019</b>. After seeing the splash/welcome screen,<br>
please click the button that reads <b>Open a local folder</b>.

<b>3.</b><br>
When the <b>Browse</b> window appears, navigate to the <b>Analysis</b> folder<br>
(the repository that you had cloned) on your local machine.<br>
Then, navigate to <b>src</b>.<br>
Click the <b>Select</b> folder button to confirm.<br>

<b>4.</b><br>
If the <b>Solution Explorer</b> view is not already open, you may open by<br>
navigating to <b>View > Solution Explorer</b>, or alternatively,<br>
you can also use the <b>Ctrl + Alt + L</b> shortcut to reveal Solution Explorer.<br>
Here, you can examine the sources imported by Visual Studio.<br>

<b>5.</b><br>
<b>We are now ready to build Analysis.</b><br>
<b>CMake</b> is used to aid in the compilation process.<br>
To build, navigate to <b>Build > Build All</b>,<br>
or you may use the <b>Ctrl + Shift + B</b> keyboard shortcut.<br>

<b>You have now built the sources for Analysis.</b><br>
You can find the build files in <b>.\Analysis\src\out\build\x64-Debug</b>

# Documentation
Documentation for <b>Analysis</b> can be found in .\Analysis\docs\<br>

The codebase is comprised of sources from:
<br>
- C++ (see .\Analysis\docs\C++)
- C#  (see .\Analysis\docs\C#)
- Python (see .\Analysis\docs\Python Docs)

# C++
The C++ portion of the codebase makes extensive use of the <b>Boost</b> libraries.<br>
You may read more about Boost at their website: https://www.boost.org/<br><br>

<u>Please note:</u>
The Boost libraries are <b>dynamically linked</b> at runtime,<br>
so you do <b>NOT</b> need to download and install Boost to build <b>Analysis</b>.<br>
Boost is included with this repository.

TBA: C# details<br>
TBA: Python details<br>
