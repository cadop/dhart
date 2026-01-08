"""
Sample input custom costs and directional limits
================================================

This walkthrough uses `sample_input.obj` (flat planes, steps, and ramps) to show:

- how to build a graph dense enough to capture steps and sloped ramps
- how to compare distance vs energy vs a blended cost
- how up-slope and down-slope limits change which directions are traversable

Custom cost tradeoff (distance vs energy vs blend)
--------------------------------------------------

.. plot::
    :context: reset

    import numpy as np
    from dhart.Examples import PathPlanningCustomCostsSampleInput as ex
    import matplotlib.pyplot as plt
    from dhart.pathfinding import DijkstraShortestPath
    from dhart.spatialstructures.cost_algorithms import (
        CalculateEnergyExpenditure,
        CostAlgorithmKeys,
    )

    obj = ex.LoadOBJ(ex.find_sample_input())
    bvh = ex.EmbreeBVH(obj, True)
    vertices = obj.vertices

    start_point = ex.find_low_start_point(bvh, vertices)
    spacing = (0.6, 0.6, 0.6)
    max_nodes = 80000
    up_step, down_step = 4.0, 4.0
    up_slope, down_slope = 85, 85
    max_step_connections = 1

    graph = ex.build_graph_for_seed(
        bvh,
        start_point,
        spacing,
        max_nodes,
        up_step,
        down_step,
        up_slope,
        down_slope,
        max_step_connections,
    )
    nodes = graph.getNodes()
    low_id = int(nodes["id"][np.argmin(nodes["z"])])
    high_id = int(nodes["id"][np.argmax(nodes["z"])])

    distance_path = DijkstraShortestPath(graph, low_id, high_id)
    CalculateEnergyExpenditure(graph)
    energy_key = CostAlgorithmKeys.ENERGY_EXPENDITURE
    energy_path = DijkstraShortestPath(graph, low_id, high_id, energy_key)

    csr = graph.CompressToCSR()
    blend_key = "EnergyDistanceBlend"
    ex.add_blended_cost(graph, csr, weight_distance=0.5, energy_key=energy_key, blended_key=blend_key)
    blend_path = DijkstraShortestPath(graph, low_id, high_id, blend_key)

    ex.plot_graph_3d(
        nodes,
        "Sample input: distance vs energy vs blended cost (3D)",
        paths=[
            ("Distance", "red", distance_path),
            ("Energy", "blue", energy_path),
            ("Blend (0.5)", "orange", blend_path),
        ],
    )
    plt.show()

Directional slope limits (up vs down)
-------------------------------------

.. plot::
    :context: close-figs

    import numpy as np
    from dhart.Examples import PathPlanningCustomCostsSampleInput as ex
    import matplotlib.pyplot as plt
    from dhart.pathfinding import DijkstraShortestPath

    obj = ex.LoadOBJ(ex.find_sample_input())
    bvh = ex.EmbreeBVH(obj, True)
    vertices = obj.vertices

    start_point = ex.find_low_start_point(bvh, vertices)
    spacing = (0.6, 0.6, 0.6)
    max_nodes = 80000
    up_step, down_step = 4.0, 4.0
    max_step_connections = 1

    limiter_graph = ex.build_graph_for_seed(
        bvh,
        start_point,
        spacing,
        max_nodes,
        up_step,
        down_step,
        10,
        85,
        max_step_connections,
    )
    if limiter_graph is None:
        alt_start = ex.find_start_point(bvh, vertices)
        limiter_graph = ex.build_graph_for_seed(
            bvh,
            alt_start,
            spacing,
            max_nodes,
            up_step,
            down_step,
            10,
            85,
            max_step_connections,
        )
    limiter_nodes = limiter_graph.getNodes()
    low_id = int(limiter_nodes["id"][np.argmin(limiter_nodes["z"])])
    high_id = int(limiter_nodes["id"][np.argmax(limiter_nodes["z"])])
    uphill_path = DijkstraShortestPath(limiter_graph, low_id, high_id)
    downhill_path = DijkstraShortestPath(limiter_graph, high_id, low_id)
    ex.plot_limiter_paths(
        limiter_nodes,
        low_id,
        high_id,
        uphill_path,
        downhill_path,
        "Up slope limited (10 deg), down slope relaxed (85 deg)",
    )
    plt.show()

.. plot::
    :context: close-figs

    import numpy as np
    from dhart.Examples import PathPlanningCustomCostsSampleInput as ex
    import matplotlib.pyplot as plt
    from dhart.pathfinding import DijkstraShortestPath

    obj = ex.LoadOBJ(ex.find_sample_input())
    bvh = ex.EmbreeBVH(obj, True)
    vertices = obj.vertices

    start_point = ex.find_low_start_point(bvh, vertices)
    spacing = (0.6, 0.6, 0.6)
    max_nodes = 80000
    up_step, down_step = 4.0, 4.0
    max_step_connections = 1

    limiter_graph = ex.build_graph_for_seed(
        bvh,
        start_point,
        spacing,
        max_nodes,
        up_step,
        down_step,
        85,
        10,
        max_step_connections,
    )
    if limiter_graph is None:
        alt_start = ex.find_start_point(bvh, vertices)
        limiter_graph = ex.build_graph_for_seed(
            bvh,
            alt_start,
            spacing,
            max_nodes,
            up_step,
            down_step,
            85,
            10,
            max_step_connections,
        )
    limiter_nodes = limiter_graph.getNodes()
    low_id = int(limiter_nodes["id"][np.argmin(limiter_nodes["z"])])
    high_id = int(limiter_nodes["id"][np.argmax(limiter_nodes["z"])])
    uphill_path = DijkstraShortestPath(limiter_graph, low_id, high_id)
    downhill_path = DijkstraShortestPath(limiter_graph, high_id, low_id)
    ex.plot_limiter_paths(
        limiter_nodes,
        low_id,
        high_id,
        uphill_path,
        downhill_path,
        "Down slope limited (10 deg), up slope relaxed (85 deg)",
    )
    plt.show()
"""

from __future__ import annotations

import os

import numpy as np
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

import dhart
from dhart.geometry import LoadOBJ
from dhart.raytracer import EmbreeBVH, IntersectForPoint
from dhart.graphgenerator import GenerateGraph
from dhart.pathfinding import DijkstraShortestPath
from dhart.spatialstructures import Graph
from dhart.spatialstructures.cost_algorithms import (
    CalculateEnergyExpenditure,
    CostAlgorithmKeys,
)


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

    raise FileNotFoundError("sample_input.obj not found in package or repo paths.")


def normalize(arr: np.ndarray) -> np.ndarray:
    min_val = float(np.min(arr))
    max_val = float(np.max(arr))
    return (arr - min_val) / (max_val - min_val + 1e-9)


def find_start_point(bvh: EmbreeBVH, vertices: np.ndarray) -> tuple[float, float, float]:
    min_xyz = vertices.min(axis=0)
    max_xyz = vertices.max(axis=0)
    max_z = float(max_xyz[2])

    candidates = []
    if len(vertices) > 0:
        z_vals = vertices[:, 2]
        target = np.quantile(z_vals, 0.5)
        idx = int(np.argmin(np.abs(z_vals - target)))
        candidates.append((float(vertices[idx, 0]), float(vertices[idx, 1])))

    candidates.extend(
        [
            ((min_xyz[0] + max_xyz[0]) / 2.0, (min_xyz[1] + max_xyz[1]) / 2.0),
            (float(min_xyz[0]), float(min_xyz[1])),
            (float(min_xyz[0]), float(max_xyz[1])),
            (float(max_xyz[0]), float(min_xyz[1])),
            (float(max_xyz[0]), float(max_xyz[1])),
        ]
    )

    rng = np.random.default_rng(123)
    if len(vertices) > 0:
        sample_count = min(25, len(vertices))
        sample_indices = rng.choice(len(vertices), size=sample_count, replace=False)
        for idx in sample_indices:
            candidates.append((float(vertices[idx, 0]), float(vertices[idx, 1])))

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


def find_seed_points(
    bvh: EmbreeBVH,
    vertices: np.ndarray,
    grid_size: int = 3,
) -> list[tuple[float, float, float]]:
    min_xyz = vertices.min(axis=0)
    max_xyz = vertices.max(axis=0)
    max_z = float(max_xyz[2])

    xs = np.linspace(min_xyz[0], max_xyz[0], grid_size)
    ys = np.linspace(min_xyz[1], max_xyz[1], grid_size)

    seeds = []
    seen = set()
    candidates = []
    for x in xs:
        for y in ys:
            origin = (float(x), float(y), max_z + 5.0)
            hit = IntersectForPoint(bvh, origin, (0, 0, -1), -1)
            if not hit:
                continue
            seed = (float(hit[0]), float(hit[1]), float(hit[2]) + 0.2)
            key = (round(seed[0], 3), round(seed[1], 3), round(seed[2], 3))
            if key not in seen:
                seeds.append(seed)
                seen.add(key)

    if len(vertices) > 0:
        z_vals = vertices[:, 2]
        for pct in (0.1, 0.5, 0.9):
            target = np.quantile(z_vals, pct)
            idx = int(np.argmin(np.abs(z_vals - target)))
            candidates.append((float(vertices[idx, 0]), float(vertices[idx, 1])))

        rng = np.random.default_rng(123)
        sample_count = min(15, len(vertices))
        sample_indices = rng.choice(len(vertices), size=sample_count, replace=False)
        for idx in sample_indices:
            candidates.append((float(vertices[idx, 0]), float(vertices[idx, 1])))

        for x, y in candidates:
            origin = (float(x), float(y), max_z + 5.0)
            hit = IntersectForPoint(bvh, origin, (0, 0, -1), -1)
            if not hit:
                continue
            seed = (float(hit[0]), float(hit[1]), float(hit[2]) + 0.2)
            key = (round(seed[0], 3), round(seed[1], 3), round(seed[2], 3))
            if key not in seen:
                seeds.append(seed)
                seen.add(key)

    if not seeds:
        seeds.append(find_start_point(bvh, vertices))

    return seeds


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


def build_graph_for_seed(
    bvh: EmbreeBVH,
    seed_point: tuple[float, float, float],
    spacing: tuple[float, float, float],
    max_nodes: int,
    up_step: float,
    down_step: float,
    up_slope: float,
    down_slope: float,
    max_step_connections: int,
):
    return GenerateGraph(
        bvh,
        seed_point,
        spacing,
        max_nodes,
        up_step,
        up_slope,
        down_step,
        down_slope,
        max_step_connections,
        cores=-1,
    )


def build_merged_graph(
    bvh: EmbreeBVH,
    seed_points: list[tuple[float, float, float]],
    spacing: tuple[float, float, float],
    max_nodes: int,
    up_step: float,
    down_step: float,
    up_slope: float,
    down_slope: float,
    max_step_connections: int,
):
    graphs = []
    for seed in seed_points:
        graph = build_graph_for_seed(
            bvh,
            seed,
            spacing,
            max_nodes,
            up_step,
            down_step,
            up_slope,
            down_slope,
            max_step_connections,
        )
        if graph is not None:
            graphs.append(graph)

    if not graphs:
        raise RuntimeError(
            "Graph generation failed; adjust start points or limiter settings."
        )

    if len(graphs) == 1:
        graph = graphs[0]
        nodes = graph.getNodes()
        csr = graph.CompressToCSR()
        return graph, nodes, csr

    merged = Graph()
    for graph in graphs:
        nodes = graph.getNodes()
        csr = graph.CompressToCSR()
        indptr = csr.indptr
        indices = csr.indices
        data = csr.data
        for parent in range(len(indptr) - 1):
            row_start = indptr[parent]
            row_end = indptr[parent + 1]
            if row_start == row_end:
                continue
            parent_coord = (
                float(nodes["x"][parent]),
                float(nodes["y"][parent]),
                float(nodes["z"][parent]),
            )
            for idx in range(row_start, row_end):
                child = int(indices[idx])
                child_coord = (
                    float(nodes["x"][child]),
                    float(nodes["y"][child]),
                    float(nodes["z"][child]),
                )
                merged.AddEdgeToGraph(parent_coord, child_coord, float(data[idx]))

    nodes = merged.getNodes()
    csr = merged.CompressToCSR()
    return merged, nodes, csr


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


def plot_graph_3d(nodes, title: str, out_path: str | None = None, paths=None) -> tuple[plt.Figure, plt.Axes]:
    fig = plt.figure(figsize=(9, 7))
    ax = fig.add_subplot(projection="3d")
    x_vals = nodes["x"]
    y_vals = nodes["y"]
    z_vals = nodes["z"]
    ax.scatter(x_vals, y_vals, z_vals, c=z_vals, s=1, alpha=0.1, cmap="viridis")
    if paths:
        for label, color, path in paths:
            if path is None:
                continue
            path_xyz = np.take(nodes[["x", "y", "z"]], path["id"])
            plot_path_with_outline(ax, path_xyz, color, label)
    setup_3d_axes(ax, x_vals, y_vals, z_vals)
    ax.set_title(title)
    if paths:
        ax.legend(loc="best")
    if out_path:
        fig.savefig(out_path, dpi=160, bbox_inches="tight")
        plt.close(fig)
    return fig, ax


def plot_limiter_paths(
    nodes,
    low_id: int,
    high_id: int,
    uphill_path,
    downhill_path,
    title: str,
    out_path: str | None = None,
) -> tuple[plt.Figure, plt.Axes]:
    fig = plt.figure(figsize=(9, 7))
    ax = fig.add_subplot(projection="3d")
    x_vals = nodes["x"]
    y_vals = nodes["y"]
    z_vals = nodes["z"]
    ax.scatter(x_vals, y_vals, z_vals, c=z_vals, s=1, alpha=0.12, cmap="viridis")

    if uphill_path is not None:
        uphill_xyz = np.take(nodes[["x", "y", "z"]], uphill_path["id"])
        plot_path_with_outline(ax, uphill_xyz, "red", "Low -> High")
    if downhill_path is not None:
        downhill_xyz = np.take(nodes[["x", "y", "z"]], downhill_path["id"])
        plot_path_with_outline(ax, downhill_xyz, "blue", "High -> Low")

    ax.scatter(
        [nodes["x"][low_id], nodes["x"][high_id]],
        [nodes["y"][low_id], nodes["y"][high_id]],
        [nodes["z"][low_id], nodes["z"][high_id]],
        color="black",
        s=30,
        marker="x",
        label="Low/High",
    )
    setup_3d_axes(ax, x_vals, y_vals, z_vals)
    ax.set_title(title)
    ax.legend(loc="best")
    if out_path:
        fig.savefig(out_path, dpi=160, bbox_inches="tight")
        plt.close(fig)
    return fig, ax


def main() -> None:
    repo_root = os.path.abspath(
        os.path.join(os.path.dirname(__file__), "..", "..", "..", "..")
    )
    output_dir = os.path.join(repo_root, "output_images")
    os.makedirs(output_dir, exist_ok=True)

    obj_path = find_sample_input()
    obj = LoadOBJ(obj_path)
    bvh = EmbreeBVH(obj, True)

    vertices = obj.vertices
    start_point = find_low_start_point(bvh, vertices)
    spacing = (0.6, 0.6, 0.6)
    max_nodes = 80000
    up_step, down_step = 4.0, 4.0
    up_slope, down_slope = 85, 85
    max_step_connections = 1

    graph = build_graph_for_seed(
        bvh,
        start_point,
        spacing,
        max_nodes,
        up_step,
        down_step,
        up_slope,
        down_slope,
        max_step_connections,
    )
    if graph is None:
        raise RuntimeError("Graph generation failed; adjust graph parameters.")
    nodes = graph.getNodes()
    csr = graph.CompressToCSR()

    node_count = len(nodes)
    low_id = int(nodes["id"][np.argmin(nodes["z"])])
    high_id = int(nodes["id"][np.argmax(nodes["z"])])

    distance_path_up = DijkstraShortestPath(graph, low_id, high_id)
    distance_path_down = DijkstraShortestPath(graph, high_id, low_id)
    if distance_path_up is None or distance_path_down is None:
        raise RuntimeError("No up/down distance path found; adjust graph parameters.")

    CalculateEnergyExpenditure(graph)
    energy_key = CostAlgorithmKeys.ENERGY_EXPENDITURE
    energy_path = DijkstraShortestPath(graph, low_id, high_id, energy_key)
    if energy_path is None:
        raise RuntimeError("No energy path found; adjust graph parameters.")

    blended_key = "EnergyDistanceBlend"
    add_blended_cost(graph, csr, weight_distance=0.5, energy_key=energy_key, blended_key=blended_key)
    blended_path = DijkstraShortestPath(graph, low_id, high_id, blended_key)
    if blended_path is None:
        raise RuntimeError("No blended path found; adjust graph parameters.")

    def path_xy(path):
        path_xyz = np.take(nodes[["x", "y", "z"]], path["id"])
        return path_xyz["x"], path_xyz["y"]

    x_nodes, y_nodes, z_nodes = nodes["x"], nodes["y"], nodes["z"]
    dist_xyz = np.take(nodes[["x", "y", "z"]], distance_path_up["id"])
    dist_down_xyz = np.take(nodes[["x", "y", "z"]], distance_path_down["id"])
    energy_xyz = np.take(nodes[["x", "y", "z"]], energy_path["id"])
    blend_xyz = np.take(nodes[["x", "y", "z"]], blended_path["id"])

    print("Distance (up) cost:", float(np.sum(distance_path_up["cost_to_next"])))
    print("Distance (down) cost:", float(np.sum(distance_path_down["cost_to_next"])))
    print("Energy cost:", float(np.sum(energy_path["cost_to_next"])))
    print("Blend cost:", float(np.sum(blended_path["cost_to_next"])))

    fig = plt.figure(figsize=(9, 7))
    ax = fig.add_subplot(projection="3d")
    ax.scatter(x_nodes, y_nodes, z_nodes, c=z_nodes, s=1, alpha=0.12, cmap="viridis")
    plot_path_with_outline(ax, dist_xyz, "red", "Distance up")
    plot_path_with_outline(ax, dist_down_xyz, "purple", "Distance down")
    plot_path_with_outline(ax, energy_xyz, "blue", "Energy")
    plot_path_with_outline(ax, blend_xyz, "orange", "Blend (0.5)")
    ax.scatter(
        [dist_xyz["x"][0], dist_xyz["x"][-1]],
        [dist_xyz["y"][0], dist_xyz["y"][-1]],
        [dist_xyz["z"][0], dist_xyz["z"][-1]],
        color="black",
        s=30,
        marker="x",
        label="Low/High",
    )
    setup_3d_axes(ax, x_nodes, y_nodes, z_nodes)
    ax.set_title("Sample input: distance vs energy vs blended cost (3D)")
    ax.legend(loc="best")
    out_path = os.path.join(output_dir, "sample_input_custom_costs.png")
    fig.savefig(out_path, dpi=160, bbox_inches="tight")
    plt.close(fig)
    print("Saved plot to:", out_path)

    graph_3d_path = os.path.join(output_dir, "sample_input_graph_3d.png")
    plot_graph_3d(
        nodes,
        f"Sample input graph (spacing={spacing}, up/down step={up_step}/{down_step}, up/down slope={up_slope}/{down_slope})",
        graph_3d_path,
        paths=[
            ("Distance up", "red", distance_path_up),
            ("Distance down", "purple", distance_path_down),
            ("Energy", "blue", energy_path),
            ("Blend", "orange", blended_path),
        ],
    )
    print("Saved plot to:", graph_3d_path)

    limiter_configs = [
        {
            "name": "limit_up_slope_only",
            "up_step": up_step,
            "down_step": down_step,
            "up_slope": 10,
            "down_slope": 85,
            "title": "Up slope limited (10 deg), down slope relaxed (85 deg)",
        },
        {
            "name": "limit_down_slope_only",
            "up_step": up_step,
            "down_step": down_step,
            "up_slope": 85,
            "down_slope": 10,
            "title": "Down slope limited (10 deg), up slope relaxed (85 deg)",
        },
    ]

    for cfg in limiter_configs:
        limiter_graph = build_graph_for_seed(
            bvh,
            start_point,
            spacing,
            max_nodes,
            cfg["up_step"],
            cfg["down_step"],
            cfg["up_slope"],
            cfg["down_slope"],
            max_step_connections,
        )
        if limiter_graph is None:
            alt_start = find_start_point(bvh, vertices)
            limiter_graph = build_graph_for_seed(
                bvh,
                alt_start,
                spacing,
                max_nodes,
                cfg["up_step"],
                cfg["down_step"],
                cfg["up_slope"],
                cfg["down_slope"],
                max_step_connections,
            )
        if limiter_graph is None:
            raise RuntimeError("Limiter graph generation failed; adjust parameters.")
        limiter_nodes = limiter_graph.getNodes()

        low_id = int(limiter_nodes["id"][np.argmin(limiter_nodes["z"])])
        high_id = int(limiter_nodes["id"][np.argmax(limiter_nodes["z"])])
        uphill_path = DijkstraShortestPath(limiter_graph, low_id, high_id)
        downhill_path = DijkstraShortestPath(limiter_graph, high_id, low_id)

        status = f"uphill={'ok' if uphill_path is not None else 'none'}, downhill={'ok' if downhill_path is not None else 'none'}"
        title = f"{cfg['title']} ({status})"
        out_path = os.path.join(output_dir, f"sample_input_{cfg['name']}.png")

        plot_limiter_paths(
            limiter_nodes,
            low_id,
            high_id,
            uphill_path,
            downhill_path,
            title,
            out_path,
        )
        print("Saved plot to:", out_path)


if __name__ == "__main__":
    main()
