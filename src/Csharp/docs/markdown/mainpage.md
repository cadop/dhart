# DHART

[TOC]

## C# Documentation

### Getting Started
- [Unity Tutorial Series](@ref UnityTutorials)


This guide will describe how to retrieve the contents of this repository,
and build dhart from source.

<h2>Please note:
dhart is currently designed to run on a PC<br>
running the Windows 10 operating system.</h2>
Also note: This guide is a 'work-in-progress' and subject to change.

<h2>Pre-requisites</h2>

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
- Microsoft Visual Studio 2019
- cmake-gui (CMake's standalone client)

<h2> Building with Microsoft Visual Studio 2019</h2>

<h3>0. Opening Visual Studio</h3>
Open <b>Microsoft Visual Studio 2019</b>. After seeing the splash/welcome screen,<br>
please click the button that reads <b>Open a local folder</b>.

<h3>1. Opening the dhart repository</h3>
When the <b>Browse</b> window appears, navigate to the <code><b>dhart</b></code> folder<br>
(the repository that you had cloned) on your local machine.<br>
Then, navigate to <code><b>src</b>.</code><br>
Click the <b>Select</b> folder button to confirm.<br>

<h3>2. Revealing the Solution Explorer</h3>
If the <b>Solution Explorer</b> view is not already open, you may open by<br>
navigating to <b>View > Solution Explorer</b>, or alternatively,<br>
you can also use the <b>Ctrl + Alt + L</b> shortcut to reveal Solution Explorer.<br>
Here, you can examine the sources imported by Visual Studio.<br>

<h3>3. Choosing a build version</h3>
There are <b>two</b> build versions for <b>dhart</b>:
- x64-Debug<br>
- x64-Release<br>

To change between build versions,<br>
navigate (move your cursor) over to the top-left portion of Visual Studio,<br>
and you will see a drop-down bar -- <b>x64-Debug</b> is selected by default.

If you want to build the <b>x64-Release</b> version of <b>dhart</b>,<br>
change the option in the drop-down bar to <b>x64-Release</b>.

<h3>4. Building the repository</h3>
<b>We are now ready to build dhart.</b><br>
<b>CMake</b> is used to aid in the compilation process.<br>

To build, navigate to <b>Build > Build All</b>,<br>
or you may use the <b>Ctrl + Shift + B</b> keyboard shortcut.<br>

<b>You have now built the sources for dhart.</b><br>
You can find the build files in <code><b>.\dhart\src\out\build\x64-Debug</b></code>

<h2>Building with CMake's standalone client (cmake-gui)</h2>
Please ensure that you have installed:
- Microsoft Visual Studio 2019 (you will get the Visual C++ compiler along with it)
- CMake

Also ensure that you have a local copy of the dhart repository.

<h3>0. Opening cmake-gui</h3>
Open the <b>CMake (cmake-gui)</b> application. (Hit the Windows key, and type 'CMake')

<h3>1. Browse source...</h3>
Hit the <b>'Browse Source...'</b> button to the right of the<br>
<b>'Where is the source code:'</b> text field.<br>

<h3>2. Selecting the source code directory</h3>
In the window that appears (titled <b>'Enter Path to Source'</b>),<br>
navigate to the local copy of <b>dhart</b> on your machine.<br>
<br>
Then, navigate to the <b>'src'</b> subdirectory. Click <b>'Select Folder'</b>.
<br>
The <b>'Where is the source code'</b> field should read:<br>
<code>C:/[YOUR_PATH_HERE]/dhart/src</code>

<h3>3. Browse build...</h3>
Hit the <b>'Browse Build...'</b> button to the right of the<br>
<b>'Where to build the binaries:'</b> text field.<br>

<h3>4. Selecting the build directory</h3>
In the window that appears (titled <b>'Enter Path to Build'</b>),<br>
navigate to the local copy of <b>dhart</b> on your machine.<br>
<br>
Then, navigate to the <b>'build'</b> subdirectory. Click <b>'Select Folder.'</b>
<br>
The <b>'Where to build the binaries'</b> field should read:<br>
<code>C:/[YOUR_PATH_HERE]/dhart/build</code>

<h3>5. Confirming your configuration</h3>
Hit the <b>'Configure'</b> button in the left-center area of the cmake-gui window.

<h3>6. Selecting compiler options</h3>
In the dialogue box that appears,<br>
for the <b>'Specify the generator for this project'</b> dropdown box,
ensure that <b>'Visual Studio 16 2019'</b> is selected.<br>

Ensure that the <b>'Use default native compilers'</b> radio button is selected.
Do not fill in the remaining text fields.<br>

Then, click <b>'Finish'</b>.

<h3>7. Building the repository/choosing a build version</h3>
As stated previously, there are <b>two</b> build versions for <b>dhart</b>:
- x64-Debug<br>
- x64-Release<br>

CMake will build the project.<br>
<b>Note: The build will appear to fail at first. </b><br>
The remainder of the directions for this step will rectify this,<br>
and you will also be able to select your desired build version.
<br><br>
In the center of the cmake-gui window, you will see<br>
a <b>'Name'</b> and <b>'Value'</b> table, in red highlighting.<br>

Next to <b>'CMAKE_CONFIGURATION_TYPES'</b>, click the text field<br>
that resides underneath the <b>'Value'</b> column.<br>

To select the build version,<br>
change the selected field to <b>Debug</b>, for the x64-Debug version,<br> 
or <b>Release</b>, for the x64-Release version, whichever is desired.<br>

Then, click <b>'Configure'</b>.

The build files generated by CMake will reside in<br>
<b><code>C:/[YOUR_PATH_HERE]/dhart/build</code></b>

# Documentation
Documentation for <b>dhart</b> can be found in .\dhart\docs\

The codebase is comprised of sources from:
<br>
- C++ (see .\dhart\docs\C++)
- C#  (see .\dhart\docs\C#)
- Python (see .\dhart\docs\Python Docs)

