"""
Energy vs distance vs blended cost
==================================

This walkthrough compares three cost models on the energy blob:

- Distance (default graph cost)
- Energy expenditure (derived cost)
- A blended cost that mixes distance and energy

The blended cost is a normalized mix of the two arrays so you can tune how
much the path favors short distance vs low energy.

.. plot::
    :context: reset

    import matplotlib.pyplot as plt
    from dhart.Examples import PathEnergyDistanceBlend as ex

    nodes, distance_path, energy_path, blend_path = ex.build_graph_and_paths()
    ex.plot_paths_3d(nodes, distance_path, energy_path, blend_path)
    plt.show()
"""

from __future__ import annotations

import numpy as np
import matplotlib.pyplot as plt

import dhart
from dhart.geometry import LoadOBJ
from dhart.graphgenerator import GenerateGraph
from dhart.pathfinding import DijkstraShortestPath
from dhart.raytracer import EmbreeBVH
from dhart.spatialstructures.cost_algorithms import (
    CalculateEnergyExpenditure,
    CostAlgorithmKeys,
)


def normalize(values: np.ndarray) -> np.ndarray:
    min_val = float(np.min(values))
    max_val = float(np.max(values))
    return (values - min_val) / (max_val - min_val + 1e-9)


def add_blended_cost(
    graph,
    csr,
    weight_distance: float,
    energy_key: str,
    blended_key: str,
) -> None:
    energy_costs = np.array(graph.GetEdgeCosts(energy_key), dtype=np.float32)
    distance_costs = csr.data.astype(np.float32)
    if energy_costs.shape[0] != distance_costs.shape[0]:
        raise RuntimeError("Energy and distance costs are different lengths.")

    blend = weight_distance * normalize(distance_costs) + (
        1.0 - weight_distance
    ) * normalize(energy_costs)

    indptr = csr.indptr
    indices = csr.indices
    for parent in range(len(indptr) - 1):
        row_start = indptr[parent]
        row_end = indptr[parent + 1]
        for idx in range(row_start, row_end):
            child = int(indices[idx])
            graph.AddEdgeToGraph(parent, child, float(blend[idx]), blended_key)


def setup_3d_axes(ax, x_vals, y_vals, z_vals) -> None:
    x_min, x_max = float(np.min(x_vals)), float(np.max(x_vals))
    y_min, y_max = float(np.min(y_vals)), float(np.max(y_vals))
    z_min, z_max = float(np.min(z_vals)), float(np.max(z_vals))
    ax.set_xlim(x_min, x_max)
    ax.set_ylim(y_min, y_max)
    ax.set_zlim(z_min, z_max)
    if hasattr(ax, "set_box_aspect"):
        ax.set_box_aspect((x_max - x_min, y_max - y_min, z_max - z_min))
    if hasattr(ax, "set_proj_type"):
        ax.set_proj_type("persp")
    ax.view_init(azim=-125, elev=22)


def plot_path_with_outline(ax, path_xyz, color: str, label: str, z_offset: float = 0.2) -> None:
    z_vals = path_xyz["z"] + z_offset
    ax.plot(
        path_xyz["x"],
        path_xyz["y"],
        z_vals,
        color="black",
        linewidth=5,
        alpha=0.9,
    )
    ax.plot(
        path_xyz["x"],
        path_xyz["y"],
        z_vals,
        color=color,
        linewidth=2.8,
        alpha=1.0,
        label=label,
    )


def build_graph_and_paths(
    spacing: tuple[float, float, float] = (2.0, 2.0, 10.0),
    max_nodes: int = 6000,
    up_step: float = 30.0,
    down_step: float = 70.0,
    up_slope: float = 60.0,
    down_slope: float = 60.0,
    max_step_connections: int = 1,
    blend_weight: float = 0.5,
):
    obj_path = dhart.get_sample_model("energy_blob_zup.obj")
    obj = LoadOBJ(obj_path)
    bvh = EmbreeBVH(obj, True)

    start_point = (-30, 0, 20)
    graph = GenerateGraph(
        bvh,
        start_point,
        spacing,
        max_nodes,
        up_step,
        up_slope,
        down_step,
        down_slope,
        max_step_connections,
        cores=-1,
    )
    if graph is None:
        raise RuntimeError("Graph generation failed; adjust graph parameters.")

    nodes = graph.getNodes()
    p_desired = np.array([[-30, 0], [30, 0]])
    start_id, end_id = graph.get_closest_nodes(p_desired, z=False)

    distance_path = DijkstraShortestPath(graph, start_id, end_id)
    if distance_path is None:
        raise RuntimeError("No distance path found; adjust graph parameters.")

    CalculateEnergyExpenditure(graph)
    energy_key = CostAlgorithmKeys.ENERGY_EXPENDITURE
    energy_path = DijkstraShortestPath(graph, start_id, end_id, energy_key)
    if energy_path is None:
        raise RuntimeError("No energy path found; adjust graph parameters.")

    csr = graph.CompressToCSR()
    blend_key = "EnergyDistanceBlend"
    add_blended_cost(graph, csr, blend_weight, energy_key, blend_key)
    blend_path = DijkstraShortestPath(graph, start_id, end_id, blend_key)
    if blend_path is None:
        raise RuntimeError("No blended path found; adjust graph parameters.")

    return nodes, distance_path, energy_path, blend_path


def plot_paths_3d(nodes, distance_path, energy_path, blend_path) -> tuple[plt.Figure, plt.Axes]:
    fig = plt.figure(figsize=(9, 7))
    ax = fig.add_subplot(projection="3d")
    x_nodes, y_nodes, z_nodes = nodes["x"], nodes["y"], nodes["z"]
    ax.scatter(x_nodes, y_nodes, z_nodes, c=z_nodes, s=1, alpha=0.12, cmap="viridis")

    distance_xyz = np.take(nodes[["x", "y", "z"]], distance_path["id"])
    energy_xyz = np.take(nodes[["x", "y", "z"]], energy_path["id"])
    blend_xyz = np.take(nodes[["x", "y", "z"]], blend_path["id"])

    plot_path_with_outline(ax, distance_xyz, "red", "Distance")
    plot_path_with_outline(ax, energy_xyz, "blue", "Energy")
    plot_path_with_outline(ax, blend_xyz, "orange", "Blend (0.5)")

    setup_3d_axes(ax, x_nodes, y_nodes, z_nodes)
    ax.set_title("Energy blob: distance vs energy vs blended cost (3D)")
    ax.legend(loc="best")
    return fig, ax


def main() -> None:
    nodes, distance_path, energy_path, blend_path = build_graph_and_paths()
    plot_paths_3d(nodes, distance_path, energy_path, blend_path)
    plt.show()


if __name__ == "__main__":
    main()
