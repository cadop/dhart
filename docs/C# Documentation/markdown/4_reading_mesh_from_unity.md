# Reading a Mesh From Unity {#MeshFromUnity}

Previous Tutorial: [The Graph Generator](@ref GeneratingAGraph)

[TOC]

## Intro

In this tutorial we'll be using the project created in the first tutorial: [Project Setup](@ref UnityProjectSetup), and we'll be using concepts covered in the previous guides.

In this guide we will cover:

- Assigning  a game object to a script so it can be referenced by code.
- Accessing the components of a Game Object
- Transforming MeshInfo instances to Z-Up

## Scene Setup

Up until this point, we haven't needed to interact with the Unity scene, aside from attaching a script to the Main Camera object that is placed by default. For us to demonstrate reading geometry from the scene, we will use the Unity editor to create a plane in the scene that's similar to the plane we previously created in code.

![Blank Scene](../assets/walkthroughs/unity/4_mesh_reading/start_point.png)

To begin, open the Unity project from [Unity Project Setup](@ref UnityProjectSetup)

### Creating the Plane

![Plane Created](../assets/walkthroughs/unity/4_mesh_reading/create_plane.png)

*Figure* **4.1**: *Creating a plane using the menu bar*

Using the menubar at the top of the screen select GameObject > 3D Object > Plane.

Once clicked, a new 10x10 plane will be created at the center of the scene window's view. The plane we created in code was centered on the origin, but this plane isn't since it was created where the scene editor's camera was looking. We must reset this plane to the origin before moving any further.

### Resetting the Plane's Position

![Transform Visible](../assets/walkthroughs/unity/4_mesh_reading/look_at_transforms.png)

*Figure* **4.2**: *View the plane in the inspector*

Left Click on the newly created plane and look at **Transform** header in the Inspector located at the right sidebar.

![Reset Position](../assets/walkthroughs/unity/4_mesh_reading/reset_position.png)

*Figure* **4.3**: *Resetting the plane's position*

Under **Transform** you can see the plane's position, rotation and scale within the scene. To set the plane's position to the origin, left on the three dots to the right of the transform header, and select *Reset Position*.

![Plane After Reset Position](../assets/walkthroughs/unity/4_mesh_reading/resetted_plane.png)

*Figure* **4.4**: *The plane centered at the scene origin*

After clicking that button, your plane should be moved to the origin like in Figure 4.4. Note that the X, Y, and Z of the plane's position in the Inspector are all set to 0. Ensure that the rest of the variables in the transform header match that of Figure 4.4. Now that we have the plane ready to go, we can begin working on the script to get its vertices and triangles.

## Writing the Script

![Blank Visual Studio Page](../assets/walkthroughs/unity/1_project_setup/visual_studio_human_factors_reference.png)

Double click on HFExampleScript that we created in project 1 to open up Visual Studio if it isn't open already. You should see a blank page like the above.

Just like the previous tutorials, we're going to declare which namespaces this script will use in the using section. For now we only need one using declaration. Add this to the top of your script like in the previous tutorials.

```{.cs}
using HumanFactors.Geometry;
```

Later you will be using different sections depending on whether you're generating a graph or firing a ray.

### Setup for Adding References Through The Unity Inspector

![PlaneReference](../assets/walkthroughs/unity/4_mesh_reading/plane_in_scene.png)

*Figure* **4.6**: *Script with a GameObject property*

There are many ways to reference a GameObject from a script, but for this example we'll be setting up our script so we can select which mesh we want to use to use from the scene in the Unity Inspector. Add A GameObject property to the script like shown on line 7 of figure 4.6.

```{.cs}
GameObject PlaneInScene;
```

Later we'll use the Unity Inspector to assign the plane in the scene to this property.

### Getting a Reference to the Mesh Held by a Specific GameObject

Before we can extract the triangles and vertices from one an instance of a Unity Mesh, we first need to get a reference to the Mesh itself. Doing this requires some understanding of Unity GameObjects and their components.

#### Background on GameObjects and Components

![PlaneReference](../assets/walkthroughs/unity/4_mesh_reading/plane_inspector.png)

*Figure* **4.6**: *Components of the Plane shown in the Unity Inspector*

Minimize Visual Studio then switch back to Unity for a moment. Click on the plane we created previously, then look at the at the Inspector on the right sidebar. It should look similar to Figure 4.6. Here you can view all information about the plane, such as its position, the material it uses, which mesh it's referencing, etc. You'll notice the inspector is split into several sections: Transform, Meshfilter, Mesh Collider. Each of these sections are a separate **Component** and the object we see in the scene is just a container for those components called a [**GameObject**](https://docs.unity3d.com/Manual/class-GameObject.html). 

As stated in the Unity Documentation:

> *GameObjects are the fundamental objects in Unity that represent characters, props and scenery. They do not accomplish much in themselves, but they act as containers for Components, which implement the real functionality.*

What this means for us is that the plane we see in the scene is not a mesh, but a GameObject that contains a [MeshFilter](https://docs.unity3d.com/ScriptReference/MeshFilter.html) that contains a Mesh, like GameObject > Mesh Filter > Mesh. To obtain the Mesh itself, we must get it from the MeshFilter component of the plane GameObject. A game object's components can be retrieved using the [GetComponent](https://docs.unity3d.com/ScriptReference/GameObject.GetComponent.html) method.

#### In Practice

In our script we will store a reference to the plane's mesh filter in a variable named `PlaneFilter` at the beginning of the script's Start() function like so:

```{.cs}
        MeshFilter PlaneFilter = PlaneInScene.GetComponent<MeshFilter>();
```

Then we can access the actual mesh carried by `PlaneFilter` by calling its .mesh property:

```{.cs}
        Mesh PlaneMesh = PlaneFilter.mesh;
```

Now we have the plane as a mesh and are ready to get the required info from it for HumanFactors.

### Getting the Vertices and Triangles

As previously stated, a mesh is comprised of an index and vertex array. To use this mesh in HumanFactors, we need to construct a MeshInfo object using these arrays. Fortunately, Unity provides a straightforward way to access the triangle indices of a mesh in a format that we can use for HumanFactors, but unfortunately the vertices only come in an array of Vector3, instead of the array of floats that we need. We will need to write a method to convert this array of Vector3 into an array of floats before we can use it in HumanFactors.

To simplify the process of converting the vertices to a suitable format, we will add a separate method called *Vector3ArrayToFloatArray* that will transform the array of Vector3 into an array of float ready for Human Factors.

On line 9, just above the Start() function, add the following method:

```{.cs}
    private float[] Vector3ArrayToFloatArray(Vector3[] vertices)
    {
        float[] return_array = new float[vertices.Length * 3];
        for (int i = 0; i < vertices.Length; i++)
        {
            int os = i * 3;
            return_array[os] = vertices[i].x;
            return_array[os + 1] = vertices[i].y;
            return_array[os + 2] = vertices[i].z;
        }

        return return_array;
    }
```

![Convert Vector3 Array](../assets/walkthroughs/unity/4_mesh_reading/flatten_vertex_array.png)

*Figure* **4.7**: *Location for Vector3ArrayToFloatArray*

Now we can call this method with the mesh's vertices as input to get an array usable with HumanFactors. Retrieve the indices and vertices from the mesh by calling its .triangles and .vertices properties, making sure to convert the array of vertices array from an array of Vector3 to an array of floats.

```{.cs}
        // Get Triangle Indexes and Vertices from the Mesh
        int[] plane_indices = PlaneMesh.triangles;
        float[] plane_vertices = Vector3ArrayToFloatArray(PlaneMesh.vertices);
```

After that, the process of constructing an instance of MeshInfo is identical to the previous tutorials:

```{.cs}
        // Send to HumanFactors
        MeshInfo PlaneMeshInfo = new MeshInfo(plane_indices, plane_vertices);
```

Before we can continue to using this mesh it's important to cover a significant difference between this mesh and the mesh we've been creating in our code.

### Transforming the Mesh From Y-Up to Z-Up

![UnityCoords](../assets/walkthroughs/unity/4_reading_mesh_from_unity/../4_mesh_reading/unity_coordinate_system.png)
![RhinoCoords](../assets/walkthroughs/unity/4_mesh_reading/rhino_coordinates.png)

*Figure* **4.8**: *Left: Unity's coordinate system. Right: HumanFactors's Coordinate system*

The Graph Generator expects geometry to be stored as if the Z-Axis were up as shown in the picture on the right. In Unity however, the Y-Axis is up, as shown in the left picture, meaning that we'll get inaccurate results if we use the meshes as is. To solve this, *MeshInfo* has a method *RotateMesh* that can rotate MeshInfo objects after they've been construction. Another class in the Geometry namespace titled *CommonRotations* contains the rotation necessary to perform this conversion.

Enter the following code to rotate the plane to the correct orientation:

```{.cs}
        //Rotate to Z-Up
        PlaneMeshInfo.RotateMesh(CommonRotations.Yup_To_Zup);
```

### Choose code From Previous Tutorial to Verify Results

Up until this point, your code should match the code below.

```{.cs}
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using HumanFactors.Geometry;

public class HFExampleScript : MonoBehaviour
{
    public GameObject PlaneInScene;
    private float[] Vector3ArrayToFloatArray(Vector3[] vertices)
    {
        float[] return_array = new float[vertices.Length * 3];
        for (int i = 0; i < vertices.Length; i++)
        {
            int os = i * 3;
            return_array[os] = vertices[i].x;
            return_array[os + 1] = vertices[i].y;
            return_array[os + 2] = vertices[i].z;
        }

        return return_array;
    }

    // Start is called before the first frame update
    void Start()
    {
        MeshFilter PlaneFilter = PlaneInScene.GetComponent<MeshFilter>();
        Mesh PlaneMesh = PlaneFilter.mesh;

        // Get Triangle Indexes and Vertices from the Mesh
        int[] plane_indices = PlaneMesh.triangles;
        float[] plane_vertices = Vector3ArrayToFloatArray(PlaneMesh.vertices);

        // Send to HumanFactors
        MeshInfo PlaneMeshInfo = new MeshInfo(plane_indices, plane_vertices);

        //Rotate to Z-Up
        PlaneMeshInfo.RotateMesh(CommonRotations.Yup_To_Zup);


    }

    // Update is called once per frame
    void Update()
    {

    }
}
```

[Screenshot of the entire code until this point](../assets/walkthroughs/unity/4_mesh_reading/end_of_getting_mesh.png)

To test whether this code can successfully read a mesh, *pick either the graph generator or the raytracer tutorial to use for testing*.  Substitute PlaneMeshInfo everywhere the code from the previous tutorials use Plane. The code you take from the other sections should be added directly after PlaneMeshInfo.RotateMesh().

The lines you should copy for each tutorial are:

- Lines 30 - 49 for the Graph Generator.
- Lines 28 - 43 for the Raytracer.

Make sure to copy over any other using declarations that the selected tutorial uses, since those are required for it to function. Once you've finished this, ***SAVE*** your script, minimize Visual Studio and switch to the Unity Editor.

## Testing the Script

Like in the previous tutorials, drag the HFExampleScript onto the Main Camera object in the scene hierarchy to set it as a component. In the previous scripts we were ready to go after this step, but here we need to assign the Plane GameObject to the script in the Unity Inspector.

### Adding References to Properties Through the Unity Inspector

![Unity Inspector New Attribute](../assets/walkthroughs/unity/4_mesh_reading/inspect_new_attribute.png)

*Figure* **4.9**: *Red circle: main camera, Highlighted: PlaneInScene Reference, Blue Circle: Assign Reference Button*

Click on the Main Camera in the left under the scene hierarchy. Notice that the HF Example Script component now has a new element under it titled *Plane In Scene*. This is the class member we created earlier, and we must assign it to the Plane we created. To assign the plane to this script, click the icon to the right of *Plane In Scene* (Circled in blue in Figure 4.9).

![Gameobject Select Window](../assets/walkthroughs/unity/4_mesh_reading/gameobject_select_window.png)

*Figure* **4.10**: *Gameobject Select Window*

A new window will appear asking you to select a game object. Double click on Plane, highlighted in Figure 4.10, then look back at the Unity Inspector.

![Gameobject Select Window](../assets/walkthroughs/unity/4_mesh_reading/inspect_with_plane.png)

*Figure* **4.11**: *Inspector With Plane*

You should now see the plane in the Unity Inspector next to PlaneInScene, like in Figure 4.11. If you see this, then you have successfully added the plane to the script as a reference.

### Comparing Output

Now, enter play mode by clicking the play button. If you have performed the above steps correctly, then your output should be identical to the output of the tutorial you chose to use in the Branching Paths section.

***Graph Generator***  :

`[(0.000,0.000,0.000), (-1.000,-1.000,0.000), (-1.000,0.000,0.000), (-1.000,1.000,0.000), (0.000,-1.000,0.000), . . . (10.000,7.000,0.000), (10.000,8.000,0.000), (10.000,9.000,0.000), (10.000,10.000,0.000)]`

***Raytracer*** :
`(0,0,1)`

If your output matches that of the tutorial you picked, then  is true then you have successfully completed this tutorial.

## Conclusion

With this you have successfully read geometry from the Unity Scene and passed it to HumanFactors. This concludes the tutorial series for this point.

If you want a more indepth look at using the Inspector to assign properties to components, see the [Official Unity Documentation](https://docs.unity3d.com/Manual/EditingValueProperties.html).