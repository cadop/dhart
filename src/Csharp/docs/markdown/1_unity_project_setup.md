
# Setting Up a Unity Project for DHART {#UnityProjectSetup}

Next Tutorial: [Using The Raytracer](@ref UsingTheRaytracer) OR [Generating A Graph](@ref GeneratingAGraph)

[TOC]

## Before we begin

In this guide we will cover the following:

- Creating a new Unity project.
- Importing DHARTAPI to a Unity project.
- Verifying that a Unity project code can reference DHARTAPI.

**IMPORTANT**: The current version of the project does not automatically link the C DLL within Unity.  The following tutorials and steps will work. However, you must build and run the unity project for the DLL to link correctly, otherwise you will recieve a `DLLNotFoundError`.  

### Prerequisites

- An installation of [Unity](https://unity3d.com/get-unity/download). This tutorial has been tested with Unity 2019.3f, but should work from 2018.3 on.
- An installation of Visual Studio with Unity integration. Instructions for installing Unity with support for Visual Studio or adding VisualStudio support to an existing installation of Unity are available [here](https://docs.microsoft.com/en-us/visualstudio/cross-platform/getting-started-with-visual-studio-tools-for-unity?view=vs-2019).
- A copy of the DHARTAPI release built with support for C#. The `bin` folder of the release should contain the following files:
  1. embree3.dll
  2. DHARTAPI.dll
  3. DHARTAPICSharp.dll
  4. msvcp140.dll
  5. System.Buffers.dll
  6. System.Memory.dll
  7. System.Runtime.CompilerServices.Unsafe.dll
  8. tbb.dll
  9. vcomp140.dll
  10. vcruntime140.dll
  11. vcruntime140_1.dll

## Creating a new project

![Image](walkthroughs/unity/1_project_setup/UnityNew.PNG)

*Figure* **1.1**: *Creating a new project in the Unity Hub*

To begin, open the Unity Hub then click the *NEW* button to create a new project.

![Choose Project Type](walkthroughs/unity/1_project_setup/unity_choose_project_type.png)

*Figure* **1.2**: *The Create a Project Dialog*

Select your desired project type, directory, and project name. For the sake of this tutorial, we will be using the *3D* project type and naming our project *DHARTAPIUnitySetup*, but you can name your project whatever you want. When done with this, press the *Create* button to create the project and wait while Unity imports all the necessary files.

![Empty Unity Scene](walkthroughs/unity/1_project_setup/blank_unity_scene.PNG)

*Figure* **1.3**: *An empty Unity scene*

Upon completion you should be greeted by an empty Unity scene.

## Importing the DHART Library

Now that we have created a blank project, we need to import the DHARTAPI binaries as assets so Unity can see and interface with them.

![Moving the bin folder into the assets directory](walkthroughs/unity/1_project_setup/bin_to_assets.png)

*Figure* **1.4**: *Dragging the DHARTAPI bin folder into the Unity Project*

If the DHARTAPI release package is zipped, unzip the contents to another location before copying it or  you will get an error when trying to import it.  Once you have the files, as shown in Figure 1.4, drag the `bin` folder from the location you extracted the release to into the Unity Editor's assets window. Once you've done this, Unity should display the *bin* folder alongside the existing *Scenes* folder.

> **NOTE**: If you accidentally drag the bin folder into the *Scenes* folder or anywhere else that would cause two copies of the same dll to exist at the same time, delete any duplicates before proceding. Multiple copies of the same DLL will cause conflicts and may result in Unity being unable to load it in the future steps. You'll know this has happened if you see the following error `Multiple plugins with the same name 'DHART_API' (found at 'Assets/Scenes/bin/DHARTAPI.dll' and 'Assets/bin/DHARTAPI.dll')...` at the bottom of the screen.
>![Duplicate DLL Error](walkthroughs/unity/1_project_setup/duplicate_dll_error.png)
>![Delete Image](walkthroughs/unity/1_project_setup/delete.png)

## Verifying Unity can reference DHARTAPI

The binaries have been added to our project as assets, but let's make sure Unity is able to reference them in code. To verify that Unity can reference the DHARTAPI binaries, we're going to create a new script and look at its references. If we did everything correctly, Unity should automatically have added DHARTAPICSharp as a reference.

![Creating a new C# script](walkthroughs/unity/1_project_setup/create_new_script.png)

*Figure* **1.5**: *Creating a new C# Script*

To create a new script, right click on a blank space in the assets window, hover over *Create*, then click *C# Script*.

![Don't click anything! Rename the script](walkthroughs/unity/2_raycast_at_plane/rename_script.png)

*Figure*  **1.6**: *Renaming a new script*

Looking in the assets window, you should notice a new script file next to Scenes directory titled NewBehaviourScript.cs by default, and the name of it is highlighted in blue indicatign that you can rename it. Rename the script to HFExampleScript.cs.

> **NOTE:** If you didn't name the file properly you can change the name of the script by right clicking on it and clicking *Rename*. This won't change the name of the class that the script contains however, so be prepared to change that when we open it. 
>![Rename Dialog](walkthroughs/unity/1_project_setup/../2_raycast_at_plane/rename_dialog.png)

![Double Click On Script](walkthroughs/unity/1_project_setup/double_click_on_script.png)

*Figure* **1.7**: *HFExampleScript.cs in the assets window*

Now that we have our new script, double click on it to open Visual studio.

![DHARTAPI referenced by your project](walkthroughs/unity/1_project_setup/visual_studio_human_factors_reference.png)

*Figure* **1.8**: *Viewing the references of a project in Visual Studio*

In Visual Studio, navigate to the *Solution Explorer* then expand the drop downs for your solution, Assembly-C-Sharp, and References. If everything was done correctly, you should see DHARTAPI C# under the references for your project, as shown in Figure 1.8.

> **NOTE**: If Mono-Develop appears instead of Visual Studio, then your installation of Unity is not set up to support Visual Studio. You can follow the above instructions from prerequisites to fix this.

> **NOTE**: If you renamed the file earlier, then your class's name may not match the name of the class in Figure 1.8. To fix this, just replace the class's name with `HFExampleScript.cs` on line 5.
> 
> ![Change Class name](walkthroughs/unity/1_project_setup/../2_raycast_at_plane/change_class_name.png)

> **NOTE:** Your version of Visual Studio may use the light theme by default like pictured below. This is purely visual and will not have any impact on this tutorial.
> ![Light Theme](walkthroughs/unity/1_project_setup/light_visual_studio.png)

If you see DHARTAPI in your project's references, then you've successfully imported the DHARTAPI library and are ready to start using the DHARTAPI library.

## Conclusion

Below is a link to the unity project created in this tutorial.

[Tutorial 1: Unity Project Setup](Tutorial%201-%20Unity%20Project.zip)

In this tutorial, we went through the process of creating unity project and adding the dhart toolkit to the project. In the following tutorials, we will use this basis for using the modules that come with the dhart toolkit.

In the next tutorial: [Using the Raytracer](@ref UsingTheRaytracer) we will use DHARTAPI to create a plane, cast a ray at it, then get the point where the ray intersected the plane.
