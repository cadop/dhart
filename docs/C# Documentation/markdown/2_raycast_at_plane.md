
# Using the RayTracer

Previous Tutorial: [Project Setup](1_unity_project_setup.md)
|
Next Tutorial: [Graph Generator](3_graph_generator.md)


## Intro

Now that we know the HumanFactors library can be referenced by our New Unity Project, lets check to see that it's working with a simple script. This script will perform the following when playmode is entered:

1) Create a Plane in HumanFactors from vertices and triangle indexes.
2) Fire a ray at the plane.
3) Print the point where the ray intersected the plane.



## Walkthrough
In the Visual Studio window you've just opened, Look at the contents of NewBehavior Script.cs It should match this image:

!["NewBehaviourScriptBlank](../assets/walkthroughs/unity/2_raycast_at_plane/blank_new_behaviour_script.png)

### Writing the Script

To reduce the length of calls into HumanFactors, we're going to declare which namespaces will be used in this script. Add the following lines to the top of the script

``` C#
using HumanFactors.Geometry;
using HumanFactors.RayTracing;
```

The top block of your script should look like this.
!["NewBehaviourScript Usings"](../assets/walkthroughs/unity/2_raycast_at_plane/add_using_delcarations.png)

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

After finished your entire code should look like this. Once you've verified this is true, make sure to save it.

![Add Using HumanFactors](../assets/walkthroughs/unity/2_raycast_at_plane/create_plane.png)

### Testing the Script

Now that we have a usable script, we need to attach it to some game object in order to run it. In the Unity Window drag *NewBehaviorScript.cs* to *Main Camera* in the scene hierarchy. Since *NewBehaviorScript* is a component of the *Main Camera*, its `Start()` function containing our sample code will be run when the editor enters play mode.

![Add Using HumanFactors](../assets/walkthroughs/unity/2_raycast_at_plane/drag_into_camera.png)

Click the play button (shown in blue below) to enter play mode, then look at your editor's console. If everything went right, then the console located at the bottom of your editor's window should show `(1,0,0)`, which is the exact point where the ray intersected the plane.

![Add Using HumanFactors](../assets/walkthroughs/unity/2_raycast_at_plane/console_show_meshinfo.png)

Here's a magnified image of the console's output shown in the red box.

!["Console Output"](../assets/walkthroughs/unity/2_raycast_at_plane/ExpectedOutput.png)

If your console's output matches this, then you're successfully completed this tutorial.

## Conclusion
Full project Link: [Tutorial 2: Casting a Ray at A Plane](../assets/walkthroughs/unity/2_raycast_at_plane/Tutorial%202%20-%20Unity%20Project.zip)


Now that we've confirmed that HumanFactors is set up and working, now we're ready to move on to a core part of the HumanFactors Library: The [Graph Generator](3_graph_generator.md). 
