#!/usr/bin/env python3
"""Generate small synthetic traces with different locality patterns."""

from __future__ import annotations

import argparse
import csv
import random
from pathlib import Path


def write_trace(path: Path, entries: list[tuple[str, int, int]]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="") as handle:
        writer = csv.writer(handle)
        writer.writerow(["op", "address", "compute_cycles"])
        for op, address, compute in entries:
            writer.writerow([op, hex(address), compute])


def compute_like(n: int, seed: int) -> list[tuple[str, int, int]]:
    rng = random.Random(seed)
    base = 0x100000
    working_set = [base + 64 * i for i in range(64)]
    entries = []
    for _ in range(n):
        address = rng.choice(working_set)
        op = "W" if rng.random() < 0.15 else "R"
        entries.append((op, address, rng.randint(4, 30)))
    return entries


def streaming(n: int, seed: int) -> list[tuple[str, int, int]]:
    rng = random.Random(seed)
    base = 0x400000
    entries = []
    for i in range(n):
        op = "W" if i % 8 == 0 else "R"
        entries.append((op, base + 64 * i, rng.randint(0, 4)))
    return entries


def ar_like(n: int, seed: int) -> list[tuple[str, int, int]]:
    rng = random.Random(seed)
    frame_base = 0x800000
    state_base = 0x200000
    state_lines = [state_base + 64 * i for i in range(128)]
    entries = []
    for i in range(n):
        if rng.random() < 0.65:
            # Frame/feature-buffer stream.
            address = frame_base + 64 * (i % 4096)
            op = "R"
        else:
            address = rng.choice(state_lines)
            op = "W" if rng.random() < 0.25 else "R"
        entries.append((op, address, rng.randint(1, 10)))
    return entries


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output-dir", type=Path, default=Path("traces/generated"))
    parser.add_argument("--length", type=int, default=20_000)
    parser.add_argument("--seed", type=int, default=7)
    args = parser.parse_args()

    write_trace(args.output_dir / "compute_like.csv", compute_like(args.length, args.seed))
    write_trace(args.output_dir / "streaming.csv", streaming(args.length, args.seed))
    write_trace(args.output_dir / "ar_like.csv", ar_like(args.length, args.seed))
    print(f"Wrote traces to {args.output_dir}")


if __name__ == "__main__":
    main()
