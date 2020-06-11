
# Using the RayTracer

Previous Tutorial: [Project Setup](1_unity_project_setup.md)
|
Next Tutorial: [Graph Generator](3_graph_generator.md)

## Table of Contents

- [Using the RayTracer](#using-the-raytracer)
  - [Table of Contents](#table-of-contents)
  - [Intro](#intro)
  - [Writing the Script](#writing-the-script)
    - [Using Declarations](#using-declarations)
    - [Creating a Plane](#creating-a-plane)
    - [Casting a Ray](#casting-a-ray)
  - [Testing](#testing)
    - [Adding the script to the camera](#adding-the-script-to-the-camera)
    - [Executing the Script](#executing-the-script)
  - [Conclusion](#conclusion)

## Intro

Now that we know the HumanFactors library can be referenced by our New Unity Project, lets check to see that it's working with a simple script. This script will perform the following when playmode is entered:

1) Create a Plane in HumanFactors from vertices and triangle indexes.
2) Cast a ray at the plane.
3) Print the point where the ray intersected the plane.

## Writing the Script

In the Visual Studio window you've just opened, Look at the contents of NewBehavior Script.cs It should match this image:

!["NewBehaviourScriptBlank](../assets/walkthroughs/unity/2_raycast_at_plane/blank_new_behaviour_script.png)

*Figure* **2.1**

### Using Declarations

To reduce the length of calls into HumanFactors, we're going to declare which namespaces will be used in this script. Add the following lines to the top of the script:

``` C#
using HumanFactors.Geometry;
using HumanFactors.RayTracing;
using HumanFactors;
```

The top block of your script should look like this.

!["NewBehaviourScript Usings"](../assets/walkthroughs/unity/2_raycast_at_plane/add_using_delcarations.png)

*Figure* **2.2**

>**Note:** While editing your code, you may notice this colored bar to the left between the line numbers and the code itself. This bar displays the changes you've made to the current document. When you make unsaved changes it turns yellow, then once you save it will turn green.

### Creating a Plane

Using HumanFactors, we will create a plane through code for our ray to intersect with. To create a mesh we require two things:

1) The (x,y,z) location of every vertex that comprises the mesh as an array of floats.
2) The Indexes for each triangle or "Face" of the mesh as an array of integers.

Here are the vertex and index arrays for a 10x10 plane on the xy plane. 
``` C#
        // Create an array of a plane's vertices and indices
        float[] plane_vertices = {
            -10f, 10f, 0f,
            -10f, -10f, 0f,
             10f, 10f, 0f,
             10f, -10f, 0f
        };
        int[] plane_indices = {
          3, 1, 0,
          2, 3, 0
        };
```
**TODO**: Replace these links with links to our documentation.

For more information on what both of these arrays mean, see out the [MeshInfo Documentation]()
We will use these arrays to construct a new **MeshInfo** object in HumanFactors.

``` C#
        MeshInfo Plane = new MeshInfo(plane_indices, plane_vertices);
```

Now that we have a plane mesh in HumanFactors, we must generate a Bounding Volume Hierarchy, or BVH, from it to use as input for the Raytracer. The BVH is an accelerated data structure that drastically reduces the time required to perform ray intersections. You can read more about the BVH in its dedicated article [Bounding Volume Hierarchy](). To generate a BVH from an instance of mesh info, just call the BVH constructor with the MeshInfo as an argument.

``` C#
        EmbreeBVH bvh = new EmbreeBVH(Plane);
```

With the BVH created, we're now ready to call the raytracer and cast a ray.

### Casting a Ray

Depending on which function you call, the EmbreeRaytracer can return 3 different types of information: the coordinates where a ray intersected the bvh, the distance from the ray's origin to the point where it intersected the BVH, or a simple true/false value for whether or not the ray intersects the mesh. For this demo we'll be using the function [IntersectForPoint]() to get the point where the ray intersects the BVH. 

We'll cast the ray from the point(1,0,1) and cast it in the direction (0,0,-1). Below is the code to do that.

``` C#
        /// Fire a ray straight down at the plane, then store the result
        Vector3D origin = new Vector3D(1, 1, 0);
        Vector3D direction = new Vector3D(0, -1, 0);
        var hitpoint = EmbreeRaytracer.IntersectForPoint(bvh, origin, direction);
```

Once hitpoint is obtained, we will print it using Debug.Log(). This will make it appear in the Unity console.

``` C#
        /// Print the x, y, and z components of the hitpoint
        Debug.Log("(" + hitpoint.x + "," + hitpoint.y + "," + hitpoint.z + ")");
```

After finished your entire code should look like this. Once you've verified this is true, ***make sure to save the script  by clicking File then Save in the top menu or pressing Ctrl + S on your keyboard.***.

![Add Using HumanFactors](../assets/walkthroughs/unity/2_raycast_at_plane/create_plane.png)

``` C#
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using HumanFactors.Geometry;
using HumanFactors.RayTracing;
using HumanFactors;

public class NewBehaviourScript : MonoBehaviour
{
    // Start is called before the first frame update
    void Start()
    {
        /// Create an array of a plane's vertices and indices
        float[] plane_vertices = {
            -20f, 0.0f, 20f,
            -20f, 0.0f, -20f,
             20f, 0.0f, 20f,
             20f, 0.0f, -20f
        };
        int[] plane_indices = { 3, 1, 0, 2, 3, 0 };

        /// Send them to HumanFactors
        MeshInfo Plane = new MeshInfo(plane_indices, plane_vertices);

        /// Generate a BVH for the RayTracer
        EmbreeBVH bvh = new EmbreeBVH(Plane);

        /// Fire a ray straight down at the plane, then store the result
        Vector3D origin = new Vector3D(1, 1, 0);
        Vector3D direction = new Vector3D(0, -1, 0);
        var hitpoint = EmbreeRaytracer.IntersectForPoint(bvh, origin, direction);

        /// Print the contents of point
        Debug.Log("(" + hitpoint.x + "," + hitpoint.y + "," + hitpoint.z + ")");
    }

    // Update is called once per frame
    void Update()
    {

    }
}
```

## Testing

Now that we have a usable script, we need to attach it to some game object in order to run it. Lets minimize the Visual Studio window and go back to the Unity Editor. 

### Adding the script to the camera

In the Unity window drag *NewBehaviorScript.cs* to *Main Camera* in the scene hierarchy.

![Add Using HumanFactors](../assets/walkthroughs/unity/2_raycast_at_plane/drag_into_camera.png)

After doing this, it may look like nothing happened, but by dragging NewBehaviorScript over the camera you've added NewBehaviorScript to the camera as a component. You can verify this by Left Clicking the camera in the scene hierarchy and looking at its components in the Inspector on the right sidebar.

![Look At Camera In Sidebar](../assets/walkthroughs/unity/2_raycast_at_plane/camera_in_sidebar.png)

At the bottom of the Inspector you should see a header for New Behaviour Script at the bottom. Once you've made sure of this you can click on empty space in the scene to deselect the camera.

### Executing the Script

Now that we've set *NewBehaviorScript* as a component of the *Main Camera*, it's `Start()` function containing our code will be run when the editor enters play mode. To enter playmode, press the play button located at the top of your screen.

![Look At Camera In Sidebar](../assets/walkthroughs/unity/2_raycast_at_plane/play_mode_button.png)

Once clicked, you will enter play mode, and the button will turn blue. 

>**NOTE:** If when entering play mode, the game window becomes full screen like below, that means you have the *Maximize On Play* option enabled. You can disable it by pressing the  *Maximize On Play* button circled in red below. For this tutorial series, we will leave this option off.
![Look At Camera In Sidebar](../assets/walkthroughs/unity/2_raycast_at_plane/playmode_maximize_on_play.png)

If everything went right, then after entering play mode the console located at the bottom of your editor's window should show `(1,0,0)`, which is the exact point where the ray intersected the plane.

![Add Using HumanFactors](../assets/walkthroughs/unity/2_raycast_at_plane/console_show_meshinfo.png)

Here's a magnified image of the console's output shown in the red box.

!["Console Output"](../assets/walkthroughs/unity/2_raycast_at_plane/ExpectedOutput.png)

If your console's output matches this, then you're successfully completed this tutorial. You can press the play button again to exit playmode.

## Conclusion

Full project Link: [Tutorial 2: Casting a Ray at A Plane](../assets/walkthroughs/unity/2_raycast_at_plane/Tutorial%202%20-%20Unity%20Project.zip)

In this tutorial we've successfully created a plane in human factors, then cast a ray at it and found the point where they have intersected. In the next tutorial we will build upon this knowledge to cover a  core part of the HumanFactors Library: The [Graph Generator](3_graph_generator.md).
