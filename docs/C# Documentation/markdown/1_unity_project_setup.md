
# Setting Up a Unity Project for Human Factors
Next Tutorial: [Project Setup](1_unity_project_setup.md)

## Table of Contents

- [Setting Up a Unity Project for Human Factors](#setting-up-a-unity-project-for-human-factors)
  - [Table of Contents](#table-of-contents)
  - [Before we begin](#before-we-begin)
    - [Prerequisites](#prerequisites)
  - [Creating a new project](#creating-a-new-project)
  - [Importing The Human Factors Library](#importing-the-human-factors-library)
  - [Verifying Unity can Reference HumanFactors](#verifying-unity-can-reference-humanfactors)
  - [Conclusion](#conclusion)

## Before we begin

 This guide will demonstrate setting up a new unity project from scratch, then importing the HumanFactors API and verifying that it can be referenced by the Unity Project.

### Prerequisites

- An installation of [Unity](https://unity3d.com/get-unity/download). This project has been tested with Unity 2019.3f, but should work from 2018.3 on. 
- An installation of Visual Studio with Unity integration. Instructions for installing Visual Studio in Unity are available [here](https://docs.microsoft.com/en-us/visualstudio/cross-platform/getting-started-with-visual-studio-tools-for-unity?view=vs-2019).
- A copy of the HumanFactors release built with support for C#. The `bin` folder should contain the following files:
  1. embree3.dll
  2. HumanFactors.dll
  3. HumanFactorsCSharp.dll
  4. msvcp140.dll
  5. System.Buffers.dll
  6. System.Memory.dll
  7. System.Runtime.CompilerServices.Unsafe.dll
  8. tbb.dll
  9. vcomp140.dll
  10. vcruntime140.dll
  11. vcruntime140_1.dll

  
## Creating a new project

Open up the Unity Hub then click the *NEW* button.
 a new unity project ![Image](../assets/walkthroughs/unity/1_project_setup/UnityNew.PNG)

 Select your desired project type, directory, and project name. For the sake of this example, we will be using the *3D* project type, and naming our project *HumanFactorsUnitySetup*. When done with this, press the Create button to create the project and wait while unity imports all the necessary info.

 ![Choose Project Type](../assets/walkthroughs/unity/1_project_setup/unity_choose_project_type.PNG)

Upon completion you should be greeted by an empty unity scene.

![Empty Unity Scene](../assets/walkthroughs/unity/1_project_setup/blank_unity_scene.PNG)

## Importing The Human Factors Library

Now that the project is set up, we need to place the binaries for HumanFactors in the assets folder so Unity can see and interface with them. Unzip the HumanFactors release package(Don't try to drag from a zipped folder you will get an error), then drag the `bin` folder into the assets window. If you don't see the assets window, then you can click the assets folder on the left side directly under the project tab.

![Moving the bin folder into the assets directory](../assets/walkthroughs/unity/1_project_setup/bin_to_assets.png)


## Verifying Unity can Reference HumanFactors

To verify that the installation is working, we're going to create a small script that constructs a sample plane. Right click on a blank space in the assets window, mouse over *Create*, then select  then select *C# Script*.

!["Creating a new C# script"](../assets/walkthroughs/unity/1_project_setup/create_new_script.png)

Double click on the newly created script to open visual studio. If Mono-Develop appears instead then you do not have visual studio support for unity, and can follow the instructions in prerequisites to install it.

!["Double Click On Script"](../assets/walkthroughs/unity/1_project_setup/double_click_on_script.png)

 In Visual Studio, navigate to the *Solution Explorer*, and expand the drop downs for your solution, Assembly-C-Sharp, and References. If everything was done correctly, you should see HumanFactors C# under the references for your project.

!["HumanFactors referenced by your project"](../assets/walkthroughs/unity/1_project_setup/visual_studio_human_factors_reference.png)

If you see HumanFactors in your project's references, then you've reached the end of this tutorial. and are ready to start using the HumanFactors library.

## Conclusion

Below is a link to the unity project created in this tutorial.


[Tutorial 1: Unity Project Setup](../assets/walkthroughs/unity/1_project_setup/Tutorial%201-%20Unity%20Project.zip)

In the next turorial: [Using the Raytracer](2_raycast_at_plane.md) we will use HumanFactors to create a plane, cast a ray at it, then get the point where the ray intersected the plane.
