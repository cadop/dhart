
# Using the RayTracer

Previous Tutorial: [Project Setup](1_unity_project_setup.md)
|
Next Tutorial: [Graph Generator](3_graph_generator.md)

## Table of Contents

- [Using the RayTracer](#using-the-raytracer)
  - [Table of Contents](#table-of-contents)
  - [Intro](#intro)
  - [Writing the Script](#writing-the-script)
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

To reduce the length of calls into HumanFactors, we're going to declare which namespaces will be used in this script. Add the following lines to the top of the script:

``` C#
using HumanFactors.Geometry;
using HumanFactors.RayTracing;
using HumanFactors;
```

The top block of your script should look like this.

!["NewBehaviourScript Usings"](../assets/walkthroughs/unity/2_raycast_at_plane/add_using_delcarations.png)
>**Note:** While editing your code, you may notice this colored bar to the left of the code between the line numbers and the code itself. This bar displays the changes you've made to the current document. When you make unsaved changes it turns yellow, then once you save it will turn green.

Now that we've declared what we're using HumanFactors, lets actually use it in our code. In the `Start()` function of new behaviour script, add the following code to create a new plane from a list of vertices/indices.

``` C#
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
