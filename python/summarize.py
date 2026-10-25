#!/usr/bin/env python3
"""Print the best sweep rows under several objectives."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("csv_path", type=Path)
    args = parser.parse_args()

    with args.csv_path.open(newline="") as handle:
        rows = list(csv.DictReader(handle))

    if not rows:
        raise SystemExit("No sweep rows found.")

    objectives = [
        ("lowest AMAT", "amat_cycles", float),
        ("fewest DRAM bytes", "bytes_from_dram", int),
        ("lowest total cycles", "total_cycles", int),
    ]

    for title, key, cast in objectives:
        best = min(rows, key=lambda row: cast(row[key]))
        print(f"\n{title}:")
        print(best)


if __name__ == "__main__":
    main()
