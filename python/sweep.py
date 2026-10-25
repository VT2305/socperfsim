#!/usr/bin/env python3
"""Run Cartesian design-space sweeps against SoCPerfSim."""

from __future__ import annotations

import argparse
import csv
import itertools
import json
import subprocess
from pathlib import Path


FLAG_MAP = {
    "l1_size": "--l1-size",
    "l1_assoc": "--l1-assoc",
    "l1_line": "--l1-line",
    "l1_policy": "--l1-policy",
    "l1_latency": "--l1-latency",
    "l2_size": "--l2-size",
    "l2_assoc": "--l2-assoc",
    "l2_line": "--l2-line",
    "l2_policy": "--l2-policy",
    "l2_latency": "--l2-latency",
    "dram_latency_cycles": "--dram-latency-cycles",
    "dram_bandwidth_gbps": "--dram-bandwidth-gbps",
    "cpu_ghz": "--cpu-ghz",
}


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--simulator", type=Path, required=True)
    parser.add_argument("--trace", type=Path, required=True)
    parser.add_argument("--config", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    config = json.loads(args.config.read_text())
    dimensions = config["sweep"]
    names = list(dimensions)
    values = [dimensions[name] for name in names]

    rows = []
    for combination in itertools.product(*values):
        params = dict(zip(names, combination))
        command = [str(args.simulator.resolve()), str(args.trace), "--json"]
        for name, value in params.items():
            if name not in FLAG_MAP:
                raise KeyError(f"Unsupported sweep key: {name}")
            command += [FLAG_MAP[name], str(value)]

        completed = subprocess.run(command, check=True, capture_output=True, text=True)
        metrics = json.loads(completed.stdout)
        rows.append({**params, **metrics})

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=list(rows[0]))
        writer.writeheader()
        writer.writerows(rows)

    print(f"Wrote {len(rows)} experiments to {args.output}")


if __name__ == "__main__":
    main()
