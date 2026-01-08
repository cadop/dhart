"""
Step and slope constraints
==========================

These examples use tiny meshes to show how step and slope limits are applied.
Fixed start points keep the results reproducible:

- step_0_5.obj bottom start: (0.5, 0.5, 1.0)
- step_1_5.obj top start: (1.5, 0.5, 3.0)
- ramp_30.obj bottom start: (0.1, 0.5, 2.0)
- ramp_30.obj top start: (0.9, 0.5, 2.0)
- ramp_60.obj bottom start: (0.1, 0.5, 3.0)

Step limits (doctest)
---------------------

>>> from dhart.Examples import GraphGeneratorConstraints as ex
>>> g_blocked = ex.build_step_graph(up_step=0.25, down_step=2.0)
>>> print(ex.z_range_str(g_blocked))
z_range=(0.00, 0.00)
>>> g_allowed = ex.build_step_graph(up_step=0.75, down_step=2.0)
>>> print(ex.z_range_str(g_allowed))
z_range=(0.00, 0.50)
>>> g_down_blocked = ex.build_down_step_graph(down_step=0.5)
>>> print(ex.z_range_str(g_down_blocked))
z_range=(1.50, 1.50)
>>> g_down_allowed = ex.build_down_step_graph(down_step=2.0)
>>> print(ex.z_range_str(g_down_allowed))
z_range=(0.00, 1.50)

Slope limits in degrees (doctest)
---------------------------------

>>> g_slope_blocked = ex.build_ramp_graph(up_slope=15, down_slope=85)
>>> print(ex.z_range_str(g_slope_blocked))
z_range=(0.06, 0.06)
>>> g_slope_allowed = ex.build_ramp_graph(up_slope=25, down_slope=85)
>>> print(ex.z_range_str(g_slope_allowed))
z_range=(0.06, 0.52)
>>> g_down_blocked = ex.build_ramp_graph(start_point=ex.RAMP_30_START_TOP, up_slope=85, down_slope=15)
>>> print(ex.z_range_str(g_down_blocked))
z_range=(0.52, 0.52)
>>> g_down_allowed = ex.build_ramp_graph(start_point=ex.RAMP_30_START_TOP, up_slope=85, down_slope=25)
>>> print(ex.z_range_str(g_down_allowed))
z_range=(0.06, 0.52)
>>> g_steep_blocked = ex.build_steep_ramp_graph(up_slope=45)
>>> print(ex.z_range_str(g_steep_blocked))
z_range=(0.17, 0.17)
>>> g_steep_allowed = ex.build_steep_ramp_graph(up_slope=70)
>>> print(ex.z_range_str(g_steep_allowed))
z_range=(0.17, 1.56)

Visualize step limits (3D)
--------------------------

.. plot::
    :context: reset

    import matplotlib.pyplot as plt
    from dhart.Examples import GraphGeneratorConstraints as ex

    g_blocked = ex.build_step_graph(up_step=0.25, down_step=2.0)
    g_allowed = ex.build_step_graph(up_step=0.75, down_step=2.0)
    step_mesh = ex.load_mesh(ex.STEP_0_5_MODEL)

    fig, axes = plt.subplots(
        1,
        2,
        figsize=(10, 4.5),
        subplot_kw={"projection": "3d"},
        constrained_layout=True,
    )
    ex.plot_nodes(axes[0], g_blocked.getNodes(), "Up step 0.25 (blocked)", mesh=step_mesh)
    ex.plot_nodes(axes[1], g_allowed.getNodes(), "Up step 0.75 (allowed)", mesh=step_mesh)
    plt.show()

.. plot::
    :context: close-figs

    import matplotlib.pyplot as plt
    from dhart.Examples import GraphGeneratorConstraints as ex

    g_blocked = ex.build_down_step_graph(down_step=0.5)
    g_allowed = ex.build_down_step_graph(down_step=2.0)
    step_mesh = ex.load_mesh(ex.STEP_1_5_MODEL)

    fig, axes = plt.subplots(
        1,
        2,
        figsize=(10, 4.5),
        subplot_kw={"projection": "3d"},
        constrained_layout=True,
    )
    ex.plot_nodes(axes[0], g_blocked.getNodes(), "Down step 0.5 (blocked)", mesh=step_mesh)
    ex.plot_nodes(axes[1], g_allowed.getNodes(), "Down step 2.0 (allowed)", mesh=step_mesh)
    plt.show()

Visualize slope limits (3D)
---------------------------

.. plot::
    :context: close-figs

    import matplotlib.pyplot as plt
    from dhart.Examples import GraphGeneratorConstraints as ex

    g_blocked_30 = ex.build_ramp_graph(up_slope=15, down_slope=85)
    g_allowed_30 = ex.build_ramp_graph(up_slope=25, down_slope=85)
    g_blocked_60 = ex.build_steep_ramp_graph(up_slope=45)
    g_allowed_60 = ex.build_steep_ramp_graph(up_slope=70)
    ramp_30_mesh = ex.load_mesh(ex.RAMP_30_MODEL)
    ramp_60_mesh = ex.load_mesh(ex.RAMP_60_MODEL)

    fig, axes = plt.subplots(
        2,
        2,
        figsize=(10, 8.5),
        subplot_kw={"projection": "3d"},
        constrained_layout=True,
    )
    ex.plot_nodes(axes[0, 0], g_blocked_30.getNodes(), "Ramp 30 deg, up 15", mesh=ramp_30_mesh)
    ex.plot_nodes(axes[0, 1], g_allowed_30.getNodes(), "Ramp 30 deg, up 25", mesh=ramp_30_mesh)
    ex.plot_nodes(axes[1, 0], g_blocked_60.getNodes(), "Ramp 60 deg, up 45", mesh=ramp_60_mesh)
    ex.plot_nodes(axes[1, 1], g_allowed_60.getNodes(), "Ramp 60 deg, up 70", mesh=ramp_60_mesh)
    plt.show()

.. plot::
    :context: close-figs

    import matplotlib.pyplot as plt
    from dhart.Examples import GraphGeneratorConstraints as ex

    g_blocked = ex.build_ramp_graph(start_point=ex.RAMP_30_START_TOP, up_slope=85, down_slope=15)
    g_allowed = ex.build_ramp_graph(start_point=ex.RAMP_30_START_TOP, up_slope=85, down_slope=25)
    ramp_mesh = ex.load_mesh(ex.RAMP_30_MODEL)

    fig, axes = plt.subplots(
        1,
        2,
        figsize=(10, 4.5),
        subplot_kw={"projection": "3d"},
        constrained_layout=True,
    )
    ex.plot_nodes(axes[0], g_blocked.getNodes(), "Ramp 30 deg, down 15", mesh=ramp_mesh)
    ex.plot_nodes(axes[1], g_allowed.getNodes(), "Ramp 30 deg, down 25", mesh=ramp_mesh)
    plt.show()
"""

from __future__ import annotations

import numpy as np
import dhart
from dhart.geometry import LoadOBJ
from dhart.graphgenerator import GenerateGraph
from dhart.raytracer import EmbreeBVH
from mpl_toolkits.mplot3d.art3d import Poly3DCollection


STEP_0_5_MODEL = "step_0_5.obj"
STEP_1_5_MODEL = "step_1_5.obj"
RAMP_30_MODEL = "ramp_30.obj"
RAMP_60_MODEL = "ramp_60.obj"

STEP_0_5_START_BOTTOM = (0.5, 0.5, 1.0)
STEP_1_5_START_TOP = (1.5, 0.5, 3.0)
RAMP_30_START_BOTTOM = (0.1, 0.5, 2.0)
RAMP_30_START_TOP = (0.9, 0.5, 2.0)
RAMP_60_START_BOTTOM = (0.1, 0.5, 3.0)

STEP_SPACING = (0.25, 0.25, 2.0)
RAMP_SPACING = (0.2, 0.2, 0.6)
DEFAULT_MAX_NODES = 500


def build_graph(
    model_name: str,
    start_point: tuple[float, float, float],
    spacing: tuple[float, float, float],
    max_nodes: int,
    up_step: float,
    up_slope: float,
    down_step: float,
    down_slope: float,
    max_step_connections: int = 1,
    min_connections: int = 1,
    cores: int = 0,
):
    obj_path = dhart.get_sample_model(model_name)
    obj = LoadOBJ(obj_path)
    bvh = EmbreeBVH(obj, True)
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
        min_connections,
        cores=cores,
    )
    if graph is None:
        raise RuntimeError("Graph generation failed; adjust parameters.")
    return graph


def graph_z_range(graph) -> tuple[float, float]:
    nodes = graph.getNodes().array
    return float(np.min(nodes["z"])), float(np.max(nodes["z"]))


def z_range_str(graph, decimals: int = 2) -> str:
    z_min, z_max = graph_z_range(graph)
    return f"z_range=({z_min:.{decimals}f}, {z_max:.{decimals}f})"


def _axis_limits(values: np.ndarray, pad: float = 0.1) -> tuple[float, float]:
    v_min, v_max = float(np.min(values)), float(np.max(values))
    if v_min == v_max:
        v_min -= pad
        v_max += pad
    return v_min, v_max


def _equalize_3d_axes(
    ax,
    x_min: float,
    x_max: float,
    y_min: float,
    y_max: float,
    z_min: float,
    z_max: float,
) -> None:
    x_center = (x_min + x_max) / 2.0
    y_center = (y_min + y_max) / 2.0
    z_center = (z_min + z_max) / 2.0
    max_range = max(x_max - x_min, y_max - y_min, z_max - z_min)
    half_range = max_range / 2.0

    ax.set_xlim(x_center - half_range, x_center + half_range)
    ax.set_ylim(y_center - half_range, y_center + half_range)
    ax.set_zlim(z_center - half_range, z_center + half_range)
    try:
        ax.set_box_aspect((1, 1, 1))
    except AttributeError:
        pass


def load_mesh(model_name: str):
    obj_path = dhart.get_sample_model(model_name)
    return LoadOBJ(obj_path)


def plot_mesh(ax, mesh, alpha: float = 0.25, color: tuple[float, float, float] = (0.7, 0.7, 0.7)) -> None:
    if not hasattr(ax, "add_collection3d"):
        return

    faces = mesh.vertices[mesh.indices]
    collection = Poly3DCollection(
        faces,
        facecolors=[(*color, alpha)],
        edgecolors=(0.2, 0.2, 0.2, min(0.35, alpha)),
        linewidths=0.2,
    )
    ax.add_collection3d(collection)


def plot_nodes(
    ax,
    nodes,
    title: str,
    mesh=None,
    mesh_alpha: float = 0.25,
) -> None:
    node_array = nodes.array if hasattr(nodes, "array") else nodes
    x_vals = node_array["x"]
    y_vals = node_array["y"]
    z_vals = node_array["z"]

    is_3d = hasattr(ax, "get_zlim")
    if is_3d:
        if mesh is not None:
            plot_mesh(ax, mesh, alpha=mesh_alpha)
        ax.scatter(
            x_vals,
            y_vals,
            z_vals,
            c=z_vals,
            s=24,
            cmap="viridis",
            alpha=0.95,
            depthshade=False,
            linewidths=0.2,
            edgecolors="black",
        )
        ax.set_xlabel("X")
        ax.set_ylabel("Y")
        ax.set_zlabel("Z")
    else:
        ax.scatter(x_vals, y_vals, c=z_vals, s=18, cmap="viridis")
    ax.set_title(title)

    if mesh is not None and hasattr(mesh, "vertices"):
        mesh_vertices = mesh.vertices
        x_min, x_max = _axis_limits(np.concatenate((x_vals, mesh_vertices[:, 0])))
        y_min, y_max = _axis_limits(np.concatenate((y_vals, mesh_vertices[:, 1])))
        z_min, z_max = _axis_limits(np.concatenate((z_vals, mesh_vertices[:, 2])))
    else:
        x_min, x_max = _axis_limits(x_vals)
        y_min, y_max = _axis_limits(y_vals)
        z_min, z_max = _axis_limits(z_vals)
    ax.set_xlim(x_min, x_max)
    ax.set_ylim(y_min, y_max)
    if is_3d:
        _equalize_3d_axes(ax, x_min, x_max, y_min, y_max, z_min, z_max)
        ax.view_init(elev=25, azim=-55)
    else:
        ax.set_aspect("equal", "box")


def build_step_graph(
    up_step: float,
    down_step: float,
    max_nodes: int = DEFAULT_MAX_NODES,
):
    return build_graph(
        STEP_0_5_MODEL,
        STEP_0_5_START_BOTTOM,
        STEP_SPACING,
        max_nodes,
        up_step,
        85.0,
        down_step,
        85.0,
    )


def build_down_step_graph(
    down_step: float,
    max_nodes: int = DEFAULT_MAX_NODES,
):
    return build_graph(
        STEP_1_5_MODEL,
        STEP_1_5_START_TOP,
        STEP_SPACING,
        max_nodes,
        2.0,
        85.0,
        down_step,
        85.0,
    )


def build_ramp_graph(
    up_slope: float,
    down_slope: float,
    start_point: tuple[float, float, float] = RAMP_30_START_BOTTOM,
    max_nodes: int = DEFAULT_MAX_NODES,
):
    return build_graph(
        RAMP_30_MODEL,
        start_point,
        RAMP_SPACING,
        max_nodes,
        2.0,
        up_slope,
        2.0,
        down_slope,
    )


def build_steep_ramp_graph(
    up_slope: float,
    down_slope: float = 85.0,
    max_nodes: int = DEFAULT_MAX_NODES,
):
    return build_graph(
        RAMP_60_MODEL,
        RAMP_60_START_BOTTOM,
        RAMP_SPACING,
        max_nodes,
        2.0,
        up_slope,
        2.0,
        down_slope,
    )
