#!/usr/bin/env python3

# SPDX-License-Identifier: MIT

import argparse
import json
from collections import defaultdict
from pathlib import Path

import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
import numpy as np


# ── Label helpers ─────────────────────────────────────────────────────────────

_SUFFIX = "_measure"

def _clean_label(name: str) -> str:
    if name.endswith(_SUFFIX):
        name = name[: -len(_SUFFIX)]
    return name.replace("_", " ")


# ── JSON loading ──────────────────────────────────────────────────────────────

def infer_library_name(path: str) -> str:
    return Path(path).stem.replace("benchmark_", "")


def load_google_benchmark_json(path: str, library_name: str) -> dict:
    with open(path, "r", encoding="utf-8") as f:
        data = json.load(f)

    case_values: dict[str, float] = {}
    case_stddev: dict[str, float] = {}
    case_errors: dict[str, str] = {}

    for entry in data.get("benchmarks", []):
        name = entry.get("name", "")
        run_type = entry.get("run_type", "")

        # Prefer aggregate mean/stddev when available
        if run_type == "aggregate":
            base_name = entry.get("run_name", name)
            agg = entry.get("aggregate_name")
            t = entry.get("real_time", entry.get("cpu_time", 0.0))
            if agg == "mean":
                case_values[base_name] = t
            elif agg == "stddev":
                case_stddev[base_name] = t
            continue

        if run_type not in ("iteration", ""):
            continue

        if entry.get("error_occurred"):
            case_errors[name] = entry.get("error_message", "error")
            continue

        if name not in case_values:
            case_values[name] = entry.get("real_time", entry.get("cpu_time", 0.0))

    return {"library": library_name, "values": case_values, "stddev": case_stddev, "errors": case_errors}


# ── Merge ─────────────────────────────────────────────────────────────────────

def merge_results(result_sets):
    merged: dict[str, dict[str, float]] = defaultdict(dict)
    stddev: dict[str, dict[str, float]] = defaultdict(dict)
    errors: dict[str, dict[str, str]] = defaultdict(dict)

    for result in result_sets:
        lib = result["library"]
        for case_name, value in result["values"].items():
            merged[case_name][lib] = value
        for case_name, sd in result.get("stddev", {}).items():
            stddev[case_name][lib] = sd
        for case_name, error in result["errors"].items():
            errors[case_name][lib] = error

    return merged, stddev, errors


# ── Plotting ──────────────────────────────────────────────────────────────────

# Dark theme colours
_BG    = "#1E1E2E"  # figure / axes background
_FG    = "#CDD6F4"  # text, ticks, labels
_GRID  = "#313244"  # grid lines
_SPINE = "#45475A"  # axis spines
_UNSUP = "#585B70"  # "unsupported" text

# Bright palette suited for dark backgrounds
_PALETTE = [
    "#89B4FA",  # blue
    "#FAB387",  # peach
    "#A6E3A1",  # green
    "#F38BA8",  # red
    "#CBA6F7",  # mauve
    "#94E2D5",  # teal
    "#F9E2AF",  # yellow
    "#89DCEB",  # sky
]

_LIB_ORDER = ["LuaBridge3Benchmark", "LuaBridgeVanillaBenchmark", "Sol3Benchmark"]
_LIB_ORDER_MAP = {lib: i for i, lib in enumerate(_LIB_ORDER)}


def plot_grouped_bars(merged: dict, stddev: dict, errors: dict, output_file: str, log_scale: bool = False) -> None:
    case_names = sorted(merged.keys())
    all_libs = {lib for cases in merged.values() for lib in cases}
    libraries = sorted(all_libs, key=lambda l: _LIB_ORDER_MAP.get(l, len(_LIB_ORDER)))

    if not case_names or not libraries:
        raise RuntimeError("No benchmark samples found to plot")

    n_cases = len(case_names)
    n_libs = len(libraries)
    clean_labels = [_clean_label(cn) for cn in case_names]

    # ── Layout ────────────────────────────────────────────────────────────────
    bar_h = 0.80
    group_h = bar_h / n_libs
    fig_h = max(10, n_cases * bar_h + 2)

    with plt.rc_context({
        "text.color":      _FG,
        "axes.labelcolor": _FG,
        "xtick.color":     _FG,
        "ytick.color":     _FG,
    }):
        fig, ax = plt.subplots(figsize=(14, fig_h))
        fig.patch.set_facecolor(_BG)
        ax.set_facecolor(_BG)

        colors = {lib: _PALETTE[i % len(_PALETTE)] for i, lib in enumerate(libraries)}
        y_positions = np.arange(n_cases, dtype=float)

        # Max value including error bars — used to size x-axis
        x_max_with_err = 1.0
        for cn in case_names:
            for lib in libraries:
                val = merged[cn].get(lib, float("nan"))
                if not np.isnan(val):
                    sd = stddev.get(cn, {}).get(lib, 0.0) or 0.0
                    x_max_with_err = max(x_max_with_err, val + sd)

        for i, library in enumerate(libraries):
            values = [merged[cn].get(library, float("nan")) for cn in case_names]
            sds    = [stddev.get(cn, {}).get(library, float("nan")) for cn in case_names]
            bar_y  = y_positions + (i - (n_libs - 1) / 2.0) * group_h

            xerr_vals = [sd if not np.isnan(sd) else 0.0 for sd in sds]
            has_errors = any(sd > 0 for sd in xerr_vals)

            ax.barh(
                bar_y,
                values,
                height=group_h * 0.85,
                color=colors[library],
                label=library,
                xerr=xerr_vals if has_errors else None,
                error_kw={"ecolor": _FG, "capsize": 3, "elinewidth": 1.2, "capthick": 1.2},
                zorder=4,
            )

            for y, val, sd in zip(bar_y, values, sds):
                if np.isnan(val):
                    ax.text(
                        0, y, "  unsupported",
                        va="center", ha="left",
                        fontsize=10, color=_UNSUP, style="italic",
                        zorder=5,
                    )
                else:
                    label = f" {val:.1f} ±{sd:.1f} ns" if not np.isnan(sd) and sd > 0 else f" {val:.1f} ns"
                    ax.text(
                        val, y, label,
                        va="center", ha="left",
                        fontsize=9, color=_FG,
                        zorder=5,
                    )

        # ── Axes ──────────────────────────────────────────────────────────────
        ax.set_yticks(y_positions)
        ax.set_yticklabels(clean_labels, fontsize=12)

        half_span = (n_libs - 1) / 2.0 * group_h + group_h * 0.425
        ax.set_ylim(-half_span, n_cases - 1 + half_span)
        ax.invert_yaxis()

        if log_scale:
            ax.set_xscale("log")
            ax.set_xlabel("Time (ns, log scale)", fontsize=13)
        else:
            ax.set_xlabel("Time (ns)", fontsize=13)

        ax.set_xlim(0, x_max_with_err * 1.20)
        ax.xaxis.grid(True, color=_GRID, linestyle="--", alpha=1.0, zorder=2)
        ax.set_axisbelow(True)
        for spine in ax.spines.values():
            spine.set_edgecolor(_SPINE)
        ax.spines["top"].set_visible(True)
        ax.spines["right"].set_visible(False)
        ax.xaxis.set_tick_params(which="both", top=True, bottom=True, labeltop=True, labelbottom=True)
        ax.tick_params(axis="x", which="both", color=_SPINE, labelsize=11)

        # ── Legend & title ────────────────────────────────────────────────────
        legend_handles = [mpatches.Patch(color=colors[lib], label=lib) for lib in libraries]
        ax.legend(
            handles=legend_handles,
            loc="upper right",
            fontsize=11,
            framealpha=1.0,
            facecolor=_SPINE,
            edgecolor=_SPINE,
            labelcolor=_FG,
        )
        ax.set_title(
            "Lua Binding Benchmarks — lower is better (ns)",
            fontsize=16, pad=10, fontweight="bold", color=_FG,
        )

        fig.subplots_adjust(left=0.22, right=0.97, top=0.97, bottom=0.03)
        plt.savefig(output_file, dpi=150, facecolor=fig.get_facecolor())
        plt.close()

    # ── Text summary ──────────────────────────────────────────────────────────
    txt_file = Path(output_file).with_suffix(".txt")
    col_w = max(len(lib) for lib in libraries) + 2
    label_w = max(len(lbl) for lbl in clean_labels) + 2

    with open(txt_file, "w", encoding="utf-8") as f:
        header = f"{'Benchmark':<{label_w}}" + "".join(f"{lib:>{col_w}}" for lib in libraries)
        f.write(header + "\n")
        f.write("-" * len(header) + "\n")
        for cn, lbl in zip(case_names, clean_labels):
            row = f"{lbl:<{label_w}}"
            for lib in libraries:
                val = merged[cn].get(lib)
                cell = f"{val:>{col_w - 3}.1f} ns" if val is not None else f"{'n/a':>{col_w}}"
                row += cell
            f.write(row + "\n")
    print(f"Saved: {txt_file}")


# ── Entry point ───────────────────────────────────────────────────────────────

def main():
    parser = argparse.ArgumentParser(
        description="Plot comparisons from Google Benchmark JSON files"
    )
    parser.add_argument(
        "--input", nargs="+", required=True,
        help="One or more Google Benchmark JSON files"
    )
    parser.add_argument(
        "--output", default="Benchmarks/benchmark_comparison.png",
        help="Output PNG file"
    )
    parser.add_argument(
        "--log", action="store_true",
        help="Use a logarithmic x-axis"
    )
    args = parser.parse_args()

    result_sets = [
        load_google_benchmark_json(path, infer_library_name(path))
        for path in args.input
    ]

    merged, stddev, errors = merge_results(result_sets)

    Path(args.output).parent.mkdir(parents=True, exist_ok=True)
    plot_grouped_bars(merged, stddev, errors, args.output, log_scale=args.log)
    print(f"Saved: {args.output}")


if __name__ == "__main__":
    main()
