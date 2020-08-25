for i in range(5):

    # Set the graph parameters
    # Test examples are failing on 1. Seems to be embree failing on edge intersection
    start_point = (1.1 + (60*i) , 1.1, 20) #offset X by 60 for each new model

    # Generate the Graph
    graph = GenerateGraph(bvh, start_point, spacing, max_nodes,
                            up_step,up_slope,down_step,down_slope,
                            max_step_connections, cores=-1)

    # Convert the graph to a CSR
    csr_graph = graph.CompressToCSR()

    # Get the nodes of the graph as a list of x,y,z,type,id tuples
    nodes = graph.getNodes()
    # Calculate view analysis on the new graph
    scores = SphericalViewAnalysisAggregate(bvh, nodes, ray_count, height,
                                            upward_fov = 20, downward_fov=20,
                                            agg_type=AggregationType.AVERAGE)

    # Plot the graph using visibility graph as the colors
    fig = plt.figure(figsize=(6,6))
    plt.scatter(nodes['x'], nodes['y'], c=scores)
    plt.show()