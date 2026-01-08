"""
Edge type tagging and step visualization
========================================

This walkthrough demonstrates how to enable extended edge tagging during graph
generation, then visualize which edges along a path are classified as steps.
The `store_edge_types` flag stores two alternate cost sets:

- `edge_type` uses the values below
- `cross_slope_tag` is 1 when the perpendicular slope exceeds the ground offset

Edge type values
----------------

- 1 = FLAT
- 2 = STEP_UP
- 3 = STEP_DOWN
- 4 = STEP_OVER
- 5 = SLOPE_UP
- 6 = SLOPE_DOWN

The example below builds a graph on `sample_input.obj`, finds a path from the
lowest to highest node, and colors each path segment by its `edge_type`.

.. plot::
    :context: reset

    import matplotlib.pyplot as plt
    from dhart.Examples import EdgeTypeWalkthrough as ex

    nodes, path_ids, edge_types, cross_tags, low_id, high_id = ex.build_edge_type_demo()
    step_edges = ex.count_step_edges(edge_types)
    cross_edges = sum(cross_tags)
    print("Step edges on path:", step_edges)
    print("Cross-slope tagged edges on path:", cross_edges)

    ex.plot_edge_type_path(nodes, path_ids, edge_types, low_id, high_id)
    plt.show()
"""

from __future__ import annotations

import os
from collections import Counter

import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
import numpy as np

import dhart
from dhart.geometry import LoadOBJ
from dhart.graphgenerator import GenerateGraph
from dhart.pathfinding import DijkstraShortestPath
from dhart.raytracer import EmbreeBVH, IntersectForPoint


EDGE_TYPE_LABELS = {
    0: "NOT_CONNECTED",
    1: "FLAT",
    2: "STEP_UP",
    3: "STEP_DOWN",
    4: "STEP_OVER",
    5: "SLOPE_UP",
    6: "SLOPE_DOWN",
}

EDGE_TYPE_COLORS = {
    0: "black",
    1: "#8a8a8a",
    2: "#d62728",
    3: "#1f77b4",
    4: "#ff7f0e",
    5: "#2ca02c",
    6: "#9467bd",
}

STEP_EDGE_TYPES = {2, 3, 4}


def find_sample_input() -> str:
    package_dir = os.path.join(os.path.dirname(dhart.__file__), "Example Models")
    candidate = os.path.join(package_dir, "sample_input.obj")
    if os.path.isfile(candidate):
        return candidate

    repo_root = os.path.abspath(
        os.path.join(os.path.dirname(__file__), "..", "..", "..", "..")
    )
    candidate = os.path.join(
        repo_root, "src", "Cpp", "tests", "Example Models", "sample_input.obj"
    )
    if os.path.isfile(candidate):
        return candidate

    return dhart.get_sample_model("energy_blob_zup.obj")


def find_start_point(bvh: EmbreeBVH, vertices: np.ndarray) -> tuple[float, float, float]:
    min_xyz = vertices.min(axis=0)
    max_xyz = vertices.max(axis=0)
    max_z = float(max_xyz[2])

    candidates = [
        ((min_xyz[0] + max_xyz[0]) / 2.0, (min_xyz[1] + max_xyz[1]) / 2.0),
        (float(min_xyz[0]), float(min_xyz[1])),
        (float(min_xyz[0]), float(max_xyz[1])),
        (float(max_xyz[0]), float(min_xyz[1])),
        (float(max_xyz[0]), float(max_xyz[1])),
    ]

    for x, y in candidates:
        origin = (x, y, max_z + 5.0)
        hit = IntersectForPoint(bvh, origin, (0, 0, -1), -1)
        if hit:
            return (float(hit[0]), float(hit[1]), float(hit[2]) + 0.2)

    raise RuntimeError("Could not find a start point above solid ground.")


def find_low_start_point(bvh: EmbreeBVH, vertices: np.ndarray) -> tuple[float, float, float]:
    if len(vertices) == 0:
        return find_start_point(bvh, vertices)

    max_z = float(vertices[:, 2].max())
    min_idx = int(np.argmin(vertices[:, 2]))
    origin = (
        float(vertices[min_idx, 0]),
        float(vertices[min_idx, 1]),
        max_z + 5.0,
    )
    hit = IntersectForPoint(bvh, origin, (0, 0, -1), -1)
    if hit:
        return (float(hit[0]), float(hit[1]), float(hit[2]) + 0.2)

    return find_start_point(bvh, vertices)


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


def build_edge_type_demo(
    spacing: tuple[float, float, float] = (0.6, 0.6, 0.6),
    max_nodes: int = 50000,
    up_step: float = 4.0,
    down_step: float = 4.0,
    up_slope: float = 85.0,
    down_slope: float = 85.0,
    max_step_connections: int = 1,
) -> tuple[np.ndarray, np.ndarray, list[int], list[int], int, int]:
    obj_path = find_sample_input()
    obj = LoadOBJ(obj_path)
    bvh = EmbreeBVH(obj, True)

    vertices = obj.vertices
    start_point = find_low_start_point(bvh, vertices)

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
        store_edge_types=True,
    )
    if graph is None:
        raise RuntimeError("Graph generation failed; adjust graph parameters.")

    nodes = graph.getNodes()
    low_id = int(nodes["id"][np.argmin(nodes["z"])])
    high_id = int(nodes["id"][np.argmax(nodes["z"])])

    path = DijkstraShortestPath(graph, low_id, high_id)
    if path is None:
        raise RuntimeError("No path found between low and high nodes.")

    path_ids = path["id"].astype(int)

    edge_types = []
    cross_tags = []
    for parent, child in zip(path_ids[:-1], path_ids[1:]):
        edge_type = int(round(graph.GetEdgeCost(parent, child, "edge_type")))
        if edge_type < 0:
            edge_type = 0
        edge_types.append(edge_type)

        cross_tag = int(round(graph.GetEdgeCost(parent, child, "cross_slope_tag")))
        if cross_tag < 0:
            cross_tag = 0
        cross_tags.append(cross_tag)

    return nodes, path_ids, edge_types, cross_tags, low_id, high_id


def count_step_edges(edge_types: list[int]) -> int:
    return sum(1 for edge_type in edge_types if edge_type in STEP_EDGE_TYPES)


def plot_edge_type_path(
    nodes,
    path_ids: np.ndarray,
    edge_types: list[int],
    low_id: int,
    high_id: int,
    z_offset: float = 0.2,
) -> tuple[plt.Figure, plt.Axes]:
    fig = plt.figure(figsize=(9, 7))
    ax = fig.add_subplot(projection="3d")

    x_vals = nodes["x"]
    y_vals = nodes["y"]
    z_vals = nodes["z"]

    ax.scatter(x_vals, y_vals, z_vals, c=z_vals, s=1, alpha=0.08, cmap="viridis")

    for idx in range(len(path_ids) - 1):
        parent = int(path_ids[idx])
        child = int(path_ids[idx + 1])
        edge_type = edge_types[idx]
        color = EDGE_TYPE_COLORS.get(edge_type, "black")
        linewidth = 2.0 if edge_type == 1 else 3.0
        alpha = 0.5 if edge_type == 1 else 1.0
        ax.plot(
            [x_vals[parent], x_vals[child]],
            [y_vals[parent], y_vals[child]],
            [z_vals[parent] + z_offset, z_vals[child] + z_offset],
            color=color,
            linewidth=linewidth,
            alpha=alpha,
        )

    ax.scatter(
        [x_vals[low_id], x_vals[high_id]],
        [y_vals[low_id], y_vals[high_id]],
        [z_vals[low_id], z_vals[high_id]],
        color="black",
        s=30,
        marker="x",
    )

    setup_3d_axes(ax, x_vals, y_vals, z_vals)
    ax.set_title("Path segments colored by edge type")

    legend_handles = []
    for edge_type in sorted(set(edge_types)):
        label = EDGE_TYPE_LABELS.get(edge_type, "UNKNOWN")
        color = EDGE_TYPE_COLORS.get(edge_type, "black")
        legend_handles.append(Line2D([0], [0], color=color, lw=3, label=label))
    if legend_handles:
        ax.legend(handles=legend_handles, loc="best")

    return fig, ax


def main() -> None:
    nodes, path_ids, edge_types, cross_tags, low_id, high_id = build_edge_type_demo()
    counts = Counter(edge_types)
    print("Edge type counts on path:")
    for key in sorted(EDGE_TYPE_LABELS):
        print(f"  {EDGE_TYPE_LABELS[key]}: {counts.get(key, 0)}")
    print("Step edges on path:", count_step_edges(edge_types))
    print("Cross-slope tagged edges:", sum(cross_tags))

    plot_edge_type_path(nodes, path_ids, edge_types, low_id, high_id)
    plt.show()


if __name__ == "__main__":
    main()
