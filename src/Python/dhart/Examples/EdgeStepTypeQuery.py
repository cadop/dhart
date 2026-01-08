"""
Query step types for edges in a generated graph.
"""

from collections import Counter
import os

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.lines import Line2D
from scipy.sparse import csr_matrix
from scipy.sparse.csgraph import dijkstra

import dhart
from dhart.geometry import LoadOBJ
from dhart.graphgenerator import GenerateGraph
from dhart.graphgenerator.graph_generator import CalculateAndStoreStepTypes
from dhart.pathfinding import DijkstraShortestPath
from dhart.raytracer import EmbreeBVH, IntersectForPoint


STEP_TYPE_NAMES = {
    0: "NOT_CONNECTED",
    1: "NONE",
    2: "UP",
    3: "DOWN",
    4: "OVER",
}
STEP_TYPE_COLORS = {
    0: "black",
    1: "#8a8a8a",
    2: "#d62728",
    3: "#1f77b4",
    4: "#ff7f0e",
}


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
    ax.set_box_aspect((x_max - x_min, y_max - y_min, z_max - z_min))
    if hasattr(ax, "set_proj_type"):
        ax.set_proj_type("persp")
    ax.view_init(azim=-125, elev=22)


def plot_step_colored_path(
    ax,
    nodes,
    path_ids: np.ndarray,
    step_types: list[int],
    z_offset: float = 0.2,
) -> None:
    x_vals = nodes["x"]
    y_vals = nodes["y"]
    z_vals = nodes["z"] + z_offset
    for idx in range(len(path_ids) - 1):
        parent = int(path_ids[idx])
        child = int(path_ids[idx + 1])
        step_type = step_types[idx]
        color = STEP_TYPE_COLORS.get(step_type, "black")
        linewidth = 2.0 if step_type == 1 else 3.2
        alpha = 0.5 if step_type == 1 else 1.0
        ax.plot(
            [x_vals[parent], x_vals[child]],
            [y_vals[parent], y_vals[child]],
            [z_vals[parent], z_vals[child]],
            color=color,
            linewidth=linewidth,
            alpha=alpha,
        )


def build_path_from_predecessors(
    predecessors: np.ndarray,
    start_id: int,
    end_id: int,
) -> np.ndarray | None:
    if start_id == end_id:
        return np.array([start_id], dtype=int)
    if predecessors[end_id] < 0:
        return None
    path = [end_id]
    node = end_id
    while node != start_id:
        node = int(predecessors[node])
        if node < 0:
            return None
        path.append(node)
    path.reverse()
    return np.array(path, dtype=int)


def main() -> None:
    repo_root = os.path.abspath(
        os.path.join(os.path.dirname(__file__), "..", "..", "..", "..")
    )
    output_dir = os.path.join(repo_root, "output_images")
    os.makedirs(output_dir, exist_ok=True)

    obj_path = find_sample_input()
    print("Using model:", obj_path)

    obj = LoadOBJ(obj_path)
    bvh = EmbreeBVH(obj, True)

    vertices = obj.vertices
    start_point = find_low_start_point(bvh, vertices)

    spacing = (0.6, 0.6, 0.6)
    max_nodes = 50000
    up_step, down_step = 4.0, 4.0
    up_slope, down_slope = 85, 85
    max_step_connections = 1

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

    CalculateAndStoreStepTypes(graph, bvh)

    nodes = graph.getNodes()
    step_csr = graph.CompressToCSR("step_type")
    step_types_csr = step_csr.data.astype(int)
    step_coo = step_csr.tocoo()
    step_types = step_coo.data.astype(int)

    counts = Counter(step_types_csr.tolist())
    print("Edge step type counts:")
    for key in sorted(STEP_TYPE_NAMES):
        print(f"  {STEP_TYPE_NAMES[key]}: {counts.get(key, 0)}")

    total_edges = step_types.size
    print("Total edges:", total_edges)
    print("Sample edges (parent -> child : step_type):")
    sample_count = min(20, total_edges)
    for idx in range(sample_count):
        parent = int(step_coo.row[idx])
        child = int(step_coo.col[idx])
        step_type = int(step_types[idx])
        print(f"  {parent} -> {child} : {STEP_TYPE_NAMES.get(step_type, 'UNKNOWN')}")

    up_mask = step_types == 2
    if np.any(up_mask):
        up_parents = step_coo.row[up_mask].astype(int)
        up_children = step_coo.col[up_mask].astype(int)
        low_id = int(up_parents[np.argmin(nodes["z"][up_parents])])
        high_id = int(up_children[np.argmax(nodes["z"][up_children])])
        path_label = "Step-preferred path"
    else:
        low_id = int(nodes["id"][np.argmin(nodes["z"])])
        high_id = int(nodes["id"][np.argmax(nodes["z"])])
        path_label = "Distance path (low -> high)"

    distance_csr = graph.CompressToCSR()
    none_multiplier = 1.4
    distance_data = distance_csr.data.astype(np.float32)
    cost_values = distance_data * np.where(
        step_types_csr == 1, none_multiplier, 1.0
    ).astype(np.float32)
    step_pref_csr = csr_matrix(
        (cost_values, distance_csr.indices, distance_csr.indptr),
        shape=distance_csr.shape,
    )
    _, predecessors = dijkstra(
        step_pref_csr, indices=low_id, return_predecessors=True
    )
    path_ids = build_path_from_predecessors(predecessors, low_id, high_id)
    if path_ids is None:
        path = DijkstraShortestPath(graph, low_id, high_id)
        if path is None:
            raise RuntimeError("No path found between selected nodes.")
        path_ids = path["id"].astype(int)
        path_label = "Distance path (low -> high)"
    path_step_types = []
    missing_step_types = 0
    for idx in range(len(path_ids) - 1):
        parent = int(path_ids[idx])
        child = int(path_ids[idx + 1])
        step_type = int(round(graph.GetEdgeCost(parent, child, "step_type")))
        if step_type < 0:
            missing_step_types += 1
            step_type = 0
        path_step_types.append(step_type)

    path_counts = Counter(path_step_types)
    print(f"Path step type counts ({path_label}):")
    for key in sorted(STEP_TYPE_NAMES):
        print(f"  {STEP_TYPE_NAMES[key]}: {path_counts.get(key, 0)}")
    step_edges = (
        path_counts.get(2, 0)
        + path_counts.get(3, 0)
        + path_counts.get(4, 0)
    )
    print(f"Step edges along path (UP/DOWN/OVER): {step_edges} (target ~15)")
    if missing_step_types:
        print("Missing step types for edges:", missing_step_types)

    fig = plt.figure(figsize=(9, 7))
    ax = fig.add_subplot(projection="3d")
    x_vals = nodes["x"]
    y_vals = nodes["y"]
    z_vals = nodes["z"]
    ax.scatter(x_vals, y_vals, z_vals, c=z_vals, s=1, alpha=0.08, cmap="viridis")
    plot_step_colored_path(ax, nodes, path_ids, path_step_types)
    ax.scatter(
        [x_vals[low_id], x_vals[high_id]],
        [y_vals[low_id], y_vals[high_id]],
        [z_vals[low_id], z_vals[high_id]],
        color="black",
        s=30,
        marker="x",
    )
    setup_3d_axes(ax, x_vals, y_vals, z_vals)
    ax.set_title(f"Path segments colored by step type ({path_label})")
    legend_handles = []
    for step_type in sorted(set(path_step_types)):
        label = STEP_TYPE_NAMES.get(step_type, "UNKNOWN")
        color = STEP_TYPE_COLORS.get(step_type, "black")
        legend_handles.append(Line2D([0], [0], color=color, lw=3, label=label))
    if legend_handles:
        ax.legend(handles=legend_handles, loc="best")
    out_path = os.path.join(output_dir, "sample_input_path_step_types.png")
    fig.savefig(out_path, dpi=160, bbox_inches="tight")
    plt.close(fig)
    print("Saved plot to:", out_path)


if __name__ == "__main__":
    main()
