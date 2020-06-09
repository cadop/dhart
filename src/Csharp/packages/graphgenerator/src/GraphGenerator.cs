using HumanFactors.SpatialStructures;
using HumanFactors.RayTracing;
using System;

namespace HumanFactors.GraphGenerator
{
    /// <summary>
    /// Generate a <see cref="Graph"/> from the accessible space in the model.
    /// </summary>
    ///<remarks> Use the graph generator to quickly map out accessible space in an environment for other analysis methods without needing to manually place nodes.</remarks>
    public static class GraphGenerator{


        /// <summary>
        /// Generate a graph of accessible space with the given settings. If no graph can be
        /// generated, null will be returned.
        /// </summary>
        /// <param name="bvh">
        /// A valid BVH to conduct the graph generation on. Geometry must be Z-Up.
        /// </param>
        /// <param name="start_point">
        /// The starting point for the graph generator. If this isn't above solid ground, no nodes will
        /// be generated.
        /// </param>
        /// <param name="spacing"> Space between nodes. Lower values will yield more nodes for a higher resolution graph. </param>
        /// <param name="max_nodes"> The maximum amount of nodes to generate. </param>
        /// <param name="up_step"> Maximum height of a step the graph can traverse. Any steps higher this will be considered inaccessible. </param>
        /// <param name="up_slope">
        /// Maximum upward slope the graph can traverse in degrees. Any slopes steeper than this
        /// will be considered inaccessible.
        /// </param>
        /// <param name="down_step">
        /// Maximum step down the graph can traverse. Any steps steeper than this will be considered inaccessible.
        /// </param>
        /// <param name="down_slope">
        /// The maximum downward slope the graph can traverse. Any slopes steeper than this will be
        /// considered inaccessible.
        /// </param>
        /// <param name="max_step_connections">
        /// Multiplier for number of children to generate for each node. Increasing this value will
        /// increase the number of edges in the graph, and as a result the amount of memory the
        /// algorithm requires.
        /// </param>
        /// <param name="core_count">
        /// Number of cores to use. -1 will use all available cores, and 0 will run a serialized version of the algorithm.
        /// </param>
        /// <returns> The resulting graph or, If no nodes were generated, null. </returns>
        /// <example>
        /// <code>
        ///MeshInfo Mesh = OBJLoader.LoadOBJ("plane.obj", CommonRotations.Yup_To_Zup);
        ///EmbreeBVH BVH = new EmbreeBVH(Mesh);
        ///
        ///Vector3D start_point = new Vector3D(0, 9000, 1);
        ///Vector3D spacing = new Vector3D(1, 1, 1);
        /// 
        /// // A standard graph with a spacing of 1 meter on each side with a maximum of 100 nodes
        ///Graph G = HumanFactors.GraphGenerator.GraphGenerator.GenerateGraph(BVH, start_point, spacing, 100);
        ///
        /// // A graph that cannot traverse stairs
        ///Graph G = HumanFactors.GraphGenerator.GraphGenerator.GenerateGraph(BVH, start_point, spacing, 100, up_step:0, downstep:0);
        ///
        /// // A graph that is able to traverse up 30 degree slopes, but can only traverse down 10 degree slopes
        ///Graph G = HumanFactors.GraphGenerator.GraphGenerator.GenerateGraph(BVH, start_point, spacing, 100, up_slope:30, down_slope:10);
        /// </code>
        /// </example>
        /// <remarks>All parameters relating to distances are in meters unless otherwise specified. </remarks>
        public static Graph GenerateGraph(
            EmbreeBVH bvh,
            Vector3D start_point,
            Vector3D spacing,
            int max_nodes = -1,
            float up_step = 0.2f,
            float up_slope = 20,
            float down_step = 0.2f,
            float down_slope = 20,
            int max_step_connections = 1,
            int core_count = -1
        ) {
           IntPtr graph_ptr =  NativeMethods.C_GenerateGraph(
                bvh.Pointer,
                start_point,
                spacing,
                max_nodes,
                up_step,
                up_slope,
                down_step,
                down_slope,
                max_step_connections,
                core_count
            );

            if (graph_ptr == IntPtr.Zero)
                return null;
            else
                return new Graph(graph_ptr);
        }
    }
}