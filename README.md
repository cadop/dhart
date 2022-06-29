Getting started:

This guide will describe how to retrieve the contents of this repository,
and build Analysis from source.

Please note:
Analysis is currently designed to run on a PC running the Windows 10
operating system.
Also note: This guide is a 'work-in-progress' and subject to change.

Pre-requisites:


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

##
0.
Open Git Bash.
Git Bash (MINGW64) begins at ~, your home directory.
You may remain here, or navigate to a directory of your choice.
In the next step, you will clone the Analysis repository.

##
1.
Type git clone git@github.com/cadop/dhart.git at the prompt
and hit ENTER. The Analysis repository will then be cloned to your local machine.

##
2.
Note that the time of this writing (06/03/2020 16:19), Analysis will build
only on the UpdateCMake branch.
To switch the branch from master to UpdateCMake,
type git checkout UpdateCMake at the prompt.
You will now be on the UpdateCMake branch.

##
3.
Open Microsoft Visual Studio 2019. After seeing the splash/welcome screen,
please click the button that reads Open a local folder.

##
4.
When the Browse window appears, navigate to the Analysis folder
(the repository that you had cloned) on your local machine.
Then, navigate to src.
Click the Select folder button to confirm.

##
5.
If the Solution Explorer view is not already open, you may open by
navigating to View > Solution Explorer, or alternatively,
you can also use the Ctrl + Alt + L shortcut to reveal Solution Explorer.
Here, you can examine the sources imported by Visual Studio.

##
6.
We are now ready to build Analysis.
CMake is used to aid in the compilation process.
To build, navigate to Build > Build All,
or you may use the Ctrl + Shift + B keyboard shortcut.

You have now built the sources for Analysis.
You can find the build files in .\Analysis\src\out\build\x64-Debug
