using DHARTAPI.Geometry;
using DHARTAPI.RayTracing;
using DHARTAPI.SpatialStructures;
using System;
using System.Linq;


/*!
    \brief Generate a graph of accessible space from a given start point.

 	\details
    The Graph Generator maps out "accessible" space on a model from a given starting point. In graphs created
    by this algorithm, node represents a point in space that a human can occupy, and each edge between nodes 
    indicates that a human can traverse from one node to another node. The Graph Generator is a powerful tool 
    for analyzing space, since the graph or nodes it outputs can be used as input to all the analysis methods 
    offered by DHARTAPI, allowing for it to be the starting point of other analysis methods within
    DHARTAPI.
    
	\par Obstacle Support
	The Graph Generator supports marking specific geometry as walkable or obstacles. Obstacle surfaces  are surfaces
	that the graph generator is not allowed to generate nodes on, while walkable surfaces are the only surfaces that the
	graph generator is permitted to generate nodes on. Depending on what arguments are first passed to the graph generator,
	it will use different rules for determining which of the inputs are obstacles and which are not. When no geometry
	ids are specified, all geometry is considered walkable. When only obstacle surfaces are specified, all geometry
	other than that in the obstacles array are considered walkable. If both an obstacle and walkable array are specified
	then obstacles will be considered inaccessible, walkable surfaces will be accessible, and all geometry not in
	either array will be considered not traversable.

    \note 
    All arguments are in meters for distances and degrees for angles
    unless otherwise specified. For all calculations, the Graph Generator assumes
    geometry is Z-Up i.e. upstep is how high the step is in the z-direction, ground
    checks are performed in the -z direction etc. 
*/
namespace DHARTAPI.GraphGenerator
{
    /*!
    \brief Generate a graph of accessible space on a mesh.

    \remarks
     Use the graph generator to quickly map out accessible space in an environment for other
     analysis methods without needing to manually place nodes.
    */
    public static class GraphGenerator
    {
        /*!
        
        \brief  Generate a graph of accessible space with the given settings. If no graph can be generated, null will be returned.

        \param bvh A valid BVH to conduct the graph generation on. Geometry must be Z-Up.
        \param start_point The starting point for the graph generator. If this isn't above solid ground, no nodes will
        be generated.
        
        \param spacing Space between nodes. Lower values will yield more nodes for a higher resolution graph.
        \param max_nodes The maximum amount of nodes to generate.
        \param up_step Maximum height of a step the graph can traverse. Any steps higher this will be considered inaccessible. 
        \param up_slope Maximum upward slope the graph can traverse in degrees. Any slopes steeper than this
         will be considered inaccessible.
        \param down_step Maximum step down the graph can traverse. Any steps steeper than this will be considered inaccessible.

        \param down_slope
         The maximum downward slope the graph can traverse. Any slopes steeper than this will be
         considered inaccessible.

        \param max_step_connections Multiplier for number of children to generate for each node. Increasing this value will
        increase the number of edges in the graph, and as a result the amount of memory the
        algorithm requires.
        
        \param min_connections The required out-degree for a node to be valid and stored.
		This must be greater than 0 and equal or less than the total connections created from max_step_connections.
		Default is 1. A value of 8 when max_step_connections=1 would be a grid.

        \param core_count Number of cores to use. -1 will use all available cores, and 0 will run a serialized version of the algorithm.
		\param walkable_id IDs of geometry to be considered as obstacles
		\param obstacle_id IDs of geometry to be considered as walkable surfaces
         
        \returns The resulting graph or, If no nodes were generated, null.
         
         \note All parameters relating to distances are in meters, and all angles are in degrees.
         \note Geometry MUST be Z-UP in order for this to work. 
         
         \code
            MeshInfo Mesh = OBJLoader.LoadOBJ("plane.obj", CommonRotations.Yup_To_Zup);
            EmbreeBVH BVH = new EmbreeBVH(Mesh);

            Vector3D start_point = new Vector3D(0, 9000, 1);
            Vector3D spacing = new Vector3D(1, 1, 1);

             // A standard graph with a spacing of 1 meter on each side with a maximum of 100 nodes
            Graph G = DHARTAPI.GraphGenerator.GraphGenerator.GenerateGraph(BVH, start_point, spacing, 100);

             // A graph that cannot traverse stairs
            Graph G = DHARTAPI.GraphGenerator.GraphGenerator.GenerateGraph(BVH, start_point, spacing, 100, up_step:0, downstep:0);

            // A graph that is able to traverse up 30 degree slopes, but can only traverse down 10
            // degree slopes
                Graph G = DHARTAPI.GraphGenerator.GraphGenerator.GenerateGraph(BVH, start_point, spacing, 100, up_slope:30, down_slope:10);
         \endcode
        */

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
            int min_connections =1,
            int core_count = -1,
            int[] obstacle_ids = null,
            int[] walkable_ids = null)
        {
            // Generate the graph in native code
            IntPtr graph_ptr = NativeMethods.C_GenerateGraph(
                 bvh.Pointer,
                 start_point,
                 spacing,
                 max_nodes,
                 up_step,
                 up_slope,
                 down_step,
                 down_slope,
                 max_step_connections,
                 min_connections,
                 core_count,
                 obstacle_ids,
                 walkable_ids
             );

            // If null was returned, return null, otherwise
            // wrap the graph pointer in the C# graph type
            if (graph_ptr == IntPtr.Zero)
                return null;
            else
                return new Graph(graph_ptr);
        }
    }
}