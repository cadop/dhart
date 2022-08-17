# Visualizing DHART Data in Unity {#UnityGraphVisualization}

Previous Tutorial: [Reading an Imported Mesh in Unity](@ref UnityImportedMesh)

[TOC]

## Intro

In this tutorial we'll be using the project created in the previous tutorial: [Reading an Imported Mesh in Unity](@ref UnityImportedMesh), and we'll be using concepts and code covered in the previous guides.

In this guide we will cover:

- Using a Unity Node Visualizer for displaying the graph

The majority of this tutorial is specific to Unity. It provides some helper functions to easily place (and fast rendering) points in space related to the DHART output.


## Node Visualizer

The main component of this tutorial is using a node visualization script. As this is Unity specific, we won't go into detail explaining the script. It has an extra feature for coloring nodes which we don't take advantage of, but could easily be used for coloring nodes by a score or weight. 

First, create a script called `NodeVisualizer.cs`. 

Copy and paste the following code into this file. 

```{.cs}
using System.Collections.Generic;
using UnityEngine;
using DHARTAPI;

namespace DHARTUTILS
{
    public enum NodeScoreType { 
        edgeConnection, 
        linear 
    };

    [RequireComponent(typeof(ParticleSystem))]
    public class NodeVisualizer : MonoBehaviour
    {

        public ParticleSystem nodeParticleSystem;
        public static NodeVisualizer instance;

        void Awake()
        {
            
            instance = this; // init singleton
            nodeParticleSystem = GetComponent<ParticleSystem>(); // Init components
        }

        public void VisualizeAllToAllNode(Vector3D position, Color color)
        {
            ParticleSystem.MainModule psMain = nodeParticleSystem.main;
            psMain.startColor = color;
            nodeParticleSystem.transform.position = new Vector3(position.x, position.y, position.z);
            nodeParticleSystem.Emit(1);
        }

        public List<Color> CreateColors(float[] scores)
        {
            // coloring
            GradientColorKey[] gradientColorKeys = new GradientColorKey[3];
            gradientColorKeys[0].color = Color.red;
            gradientColorKeys[0].time = 0.0f;
            gradientColorKeys[1].color = Color.green;
            gradientColorKeys[1].time = 0.5f;
            gradientColorKeys[2].color = Color.blue;
            gradientColorKeys[2].time = 1.0f;

            GradientAlphaKey[] gradientAlphaKeys = new GradientAlphaKey[3];
            gradientAlphaKeys[0].alpha = 1.0f;
            gradientAlphaKeys[0].time = 0.0f;
            gradientAlphaKeys[1].alpha = 1.0f;
            gradientAlphaKeys[1].time = 1.0f;
            gradientAlphaKeys[2].alpha = 1.0f;
            gradientAlphaKeys[2].time = 1.0f;

            Gradient gradient = new Gradient();
            gradient.colorKeys = gradientColorKeys;
            gradient.alphaKeys = gradientAlphaKeys;

            // get max score
            float maxScore = 0.0f;
            for (int i = 0; i < scores.Length; i++)
            {
                float currScore = scores[i];
                if (currScore > maxScore)
                    maxScore = currScore;
            }

            float[] clampedScores = new float[scores.Length];
            List<Color> colors = new List<Color>();
            for (int i = 0; i < scores.Length; i++)
            {
                float value = scores[i];

                if (float.IsInfinity(value) || float.IsNaN(value))
                {
                    Debug.Log("Aggregated edge cost result equaled infinity or NaN." +
                        "Setting its value in the array to 0 to avoid crashes.");
                    continue;
                }
                clampedScores[i] = Mathf.Clamp(scores[i], 0, maxScore) / maxScore;
                Color color = gradient.Evaluate(clampedScores[i]);
                colors.Add(color);
            }
            return colors;
        }

    }

}


```

### Setting the Particle Property

First we will make a new material that will be used by the particle system. Navigate (or create) a folder in the Assets called Materials.

Right click and Create->Material


![Creating Material](walkthroughs/unity/6_graph_visualization/create_material.JPG)


Rename the material to "Node Material" and select it in the Assets browser.  

In the Inspector, match the values shown in the following image. Specifically, the `Shader` dropdown should be set to `UI`->`Default`.

![Material Properties](walkthroughs/unity/6_graph_visualization/node_material.JPG)


- Make an empty game object in the scene hierarchy called "NodeViz". 

- Drag and drop the `NodeVisualization.cs` script onto the NodeViz game object.


![Particle System](walkthroughs/unity/6_graph_visualization/scene_setup_1.JPG)

Now that there is a particle system, go through each setting and make sure it matches the settings shown in the images below.  Note that the Renderer has a material, which is what you will use to assign the material created in the step above.

![Particle System](walkthroughs/unity/6_graph_visualization/particle_system_1.JPG)


![Particle System](walkthroughs/unity/6_graph_visualization/particle_system_2.JPG)



## Generating a Graph

At this point we assume you have followed the previous tutorials, and so much of this code should be easy enough to follow along.  If you have any questions please post on the github Discussions page. 


- Create a script called `DHART_Graph.cs`

Drag this script onto the Main Camera game object


This script is largely a combination of the previous tutorials with a few differences.  

- We take a gameobject for a starting point that can be used to interactively change where the graph starts.  
- The graph nodes are copied to C# and the coordinates are converted back to Unity.
- The converted nodes are visualized by passing to a function we call `PlotNodes()`.


```{.cs}
using UnityEngine;
using System.Linq;

using DHARTAPI;
using DHARTAPI.RayTracing;
using DHARTAPI.Geometry;
using DHARTAPI.SpatialStructures;
using DHARTAPI.GraphGenerator;
using DHARTUTILS;


public class DHART_Graph : MonoBehaviour
{
    EmbreeBVH WalkableBVH; // a BVH for walkable space

    public int max_nodes = 100;
    public GameObject StartPoint;

    public Graph graph; // Most recently generated graph
    public NodeList node_list; // nodes of the most recently generated graph
    public Node[] nodes = null; // Nodes of the most recently generated graph copied out of the graph
                                
    public GameObject MeshParent;

    // A node visualizer
    private NodeVisualizer nodeVisualizer;

    // Start is called before the first frame update
    void Start()
    {

        nodeVisualizer = NodeVisualizer.instance;

        // Only enable the one at mesh parent index
        MeshParent.SetActive(true);

        // Get every mesh from this parent
        var meshes = MeshUtils.GetMeshesFromParent(MeshParent);

        // Send them to the humanfactors manager to be converted to meshinfo for the bvh
        this.CreateSeperateBVH(meshes);

        // Get the start point of the graph from unity game object 
        Vector3 start_point = this.StartPoint.transform.position;
        // Convert to right hand z-up
        start_point = new Vector3(start_point.x * -1, start_point.z * -1, start_point.y);
        // generate graph
        this.CreateGraph(start_point);

        ///// VISUALIZATION 

        // Pull nodes out of the graph
        Node[] graph_nodes = this.graph.getNodes().CopyArray();

        // Nodes are Z up and Right-hand.        
        Vector3[] nodes_vec = TransformUtils.Node_to_Vector(graph_nodes);

        // Various methods for moving around nodes
        Vector3[] nodes_Yup = TransformUtils.PointList_Z2Y(nodes_vec); // Convert to Y up
        Vector3[] nodes_Lcoor = TransformUtils.PointList_R2L(nodes_Yup); // Convert to left hand

        // Visualize the different node transformations

        PlotNodes(nodes_Lcoor, 0, 0, 1); // Show rotated Y up and then left hand coordinate as blue

    }


    // Create A BVH for casting rays, and a BVH for generating the graph. 
    public void CreateSeperateBVH(Mesh[] global)
    {
        // Create meshinfo for every mesh in the graph, and rotate them from y-up to z-up
        MeshInfo[] Meshes = new MeshInfo[global.Length];
        for (int i = 0; i < Meshes.Length; i++)
        {
            var tris = global[i].triangles;
            var vert_array = MeshUtils.FlattenVerticeArray(global[i].vertices, true);
            var mesh_name = global[i].name;

            // Make sure there are vertices in this mesh
            if(vert_array.Length > 0){
                Meshes[i] = new MeshInfo(tris, vert_array, mesh_name);
            }
        }

        // Remove nulls
        Meshes = Meshes.Where(c => c != null).ToArray();

        // Filter BVHs using substring matching and some set logic. 
        MeshInfo[] Doors = Meshes.Where(mesh => (mesh.name.ToLower().Contains("door"))).ToArray();
        MeshInfo[] Windows = Meshes.Where(mesh => (mesh.name.ToLower().Contains("window"))).ToArray();
        MeshInfo[] Global = Meshes.Except(Doors).Except(Windows).ToArray();

        var walkableMesh = Global.Union(Doors).ToArray();

        // Make sure the final mesh exists
        if (walkableMesh.Length > 0){
            this.WalkableBVH = new EmbreeBVH(walkableMesh);
        }
    }

    private void PlotNodes(Vector3[] nodes, int r = 0, int g = 0, int b = 0)
    {
        Color color = new Color(r, g, b);
        for (int i = 0; i < nodes.Length; i++)
        {
            Vector3D node = new Vector3D(nodes[i].x, nodes[i].y, nodes[i].z);
            nodeVisualizer.VisualizeAllToAllNode(node, color);
        }
    }

    // Create a new path at the start point. All of the inputs are hardcoded for the sake of time
    public bool CreateGraph(Vector3 StartPoint)
    {
        // If no BVH has been generated, then return so we don't crash
        if (this.WalkableBVH == null) return false;

        // Generate a graph
        Vector3D start = MeshUtils.ConvertToVector3D(StartPoint);
        Vector3D spacing = new Vector3D(0.2f, 0.2f, 1.7f);

        Debug.Log("Converted start of graph: " + start);

        var graph_watch = System.Diagnostics.Stopwatch.StartNew();
        this.graph = GraphGenerator.GenerateGraph(this.WalkableBVH, start, spacing, this.max_nodes, up_step: 0.4f, down_step: 0.4f);
        graph_watch.Stop();

        // if it failed return false
        if (this.graph == null)
            return false;

        else
        {
            // Otherwise update our node arrays and compress the graph
            Debug.Log("Graph Generated " + graph.NumNodes().ToString() + " nodes in " + graph_watch.ElapsedMilliseconds.ToString() + "ms");
            this.node_list = graph.getNodes();
            graph.CompressToCSR();
            this.nodes = node_list.CopyArray();
            return true;
        }
    }
}

```

Once this script is created, select the Main Camera, which should have your script, drag the empty game object called GraphStart and the Mesh model to the "Mesh Parent" space.


### Helper Scripts

- Create a script called `MeshUtils.cs`
- Create a script called `TransformUtils.cs`

These do not need to be assigned to any object.


The `MeshUtils.cs` continues from the previous script:

```{.cs}

using DHARTAPI;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Rendering;


public static class MeshUtils
{

	// Just combine all the submeshes into one big mesh, transforming their
	// vertices by the passed transformation matrix.
	static public Mesh CombineSubmeshes(Mesh mesh, Matrix4x4 matrix)
	{
		List<CombineInstance> ci = new List<CombineInstance>();
		for (int i = 0; i < mesh.subMeshCount; i++)
		{
			CombineInstance combine = new CombineInstance();
			combine.mesh = mesh;
			combine.transform = matrix;
			combine.subMeshIndex = i;

			ci.Add(combine);
		}
		Mesh out_mesh = new Mesh();
		out_mesh.indexFormat = IndexFormat.UInt32;
		out_mesh.CombineMeshes(ci.ToArray(), true, true);
		out_mesh.name = mesh.name;
		return out_mesh;
	}

	public static Mesh[] GetMeshesFromParent(GameObject MeshParent)
	{
		// Get the meshrenders from the parent
		var mesh_renders = MeshParent.GetComponentsInChildren<MeshFilter>();

		List<Mesh> out_meshes = new List<Mesh>();
		// Get the transforms and renderer for each mesh
		foreach (var filter in mesh_renders)
		{
			out_meshes.Add(MeshUtils.CombineSubmeshes(filter.mesh, filter.transform.localToWorldMatrix));
		}

		return out_meshes.ToArray();
	}

	public static float[] FlattenVerticeArray(Vector3[] inds, bool convert_coord = false)
	{
		float[] return_array = new float[inds.Length * 3];
		for (int i = 0; i < inds.Length; i++)
		{
			int os = i * 3;
			if (convert_coord)
			{
				return_array[os] = -1 * inds[i].x;
				return_array[os + 1] = -1 * inds[i].z;
				return_array[os + 2] = inds[i].y;
			}
			else
			{
				return_array[os] = inds[i].x;
				return_array[os + 1] = inds[i].y;
				return_array[os + 2] = inds[i].z;
			}
		}

		return return_array;
	}

	public static Vector3D ConvertToVector3D(Vector3 PointToConvert, bool convert_coords = false)
	{
		// CHANGES COORD
		// Left hand  Y-UP to Right hand Z-UP
		if (convert_coords) return new Vector3D(-1 * PointToConvert.x, -1 * PointToConvert.z, PointToConvert.y);

		else return new Vector3D(PointToConvert.x, PointToConvert.y, PointToConvert.z);
	}

}



```

The `TransformUtils.cs` has a few helper functions for coordinate conversion as well as datatypes. The code to copy is:

```{.cs}

using System.Collections.Generic;
using UnityEngine;
using DHARTAPI;
using DHARTAPI.SpatialStructures;

namespace DHARTUTILS
{
    static class TransformUtils
    {

        public static Vector3[] PointList_Z2Y(Vector3[] nodes_in)
        {
            Vector3[] nodes = new Vector3[nodes_in.Length];
            // Rotate the point from Z up coordinates to Y up coordinates
            for (int i = 0; i < nodes.Length; i++)
            {
                nodes[i].x = nodes_in[i].x; // Rotating around X, so X does not change
                nodes[i].y = nodes_in[i].z; // Y is up, so Y replaces Z
                nodes[i].z = -1 * nodes_in[i].y; // Z moves to negative Y
            }
            return nodes;
        }

        public static Vector3[] PointList_R2L(Vector3[] nodes_in)
        {
            Vector3[] nodes = new Vector3[nodes_in.Length];
            // Flip the X axis to go from right hand coordinates to left hand coordinates
            for (int i = 0; i < nodes.Length; i++)
            {
                nodes[i].x = -1 * nodes_in[i].x;
                nodes[i].y = nodes_in[i].y;
                nodes[i].z = nodes_in[i].z;
            }
            return nodes;
        }

        public static List<Vector3D> ToVector3D(Vector3[] nodes_in)
        {
            List<Vector3D> v3d_nodes = new List<Vector3D>();
            // Iterate through nodes
            for (int i = 0; i < nodes_in.Length; i++)
            {
                // Create a Vector3D
                Vector3D nodePos = new Vector3D(nodes_in[i].x, nodes_in[i].y, nodes_in[i].z);
                // Add it to the list
                v3d_nodes.Add(nodePos);
            }
            return v3d_nodes;
        }

        public static Vector3[] Node_to_Vector(Node[] nodesList)
        {
            // Convert list of nodes to list of vectors
            Vector3[] node_vec = new Vector3[nodesList.Length];
            for (int i = 0; i < nodesList.Length; i++)
                node_vec[i] = new Vector3(nodesList[i].x, nodesList[i].y, nodesList[i].z);

            return node_vec;
        }

    }
} // End  Namespace


```

- Create an empty game object called `GraphStart` 

Place this object above the geometry in which you will generate a graph. 

