#!/usr/bin/env python3
"""Fail with exit code 1 if any benchmark regressed beyond the given threshold.

Compares cpu_time from two Google Benchmark JSON result files.
Benchmarks present in only one file are silently skipped (new or removed).

Usage:
    python3 bench_check_regression.py \
        --baseline old.json --current new.json [--threshold 5]
"""

import argparse
import json
import sys


def load_results(path):
    """Return a dict name→benchmark for iteration or mean-aggregate entries."""
    with open(path) as f:
        data = json.load(f)
    benchmarks = data.get("benchmarks", [])

    # Prefer raw iteration results (single run, no --benchmark_repetitions).
    by_name = {b["name"]: b for b in benchmarks if b.get("run_type") == "iteration"}
    if by_name:
        return by_name

    # Fall back to mean aggregates (produced when --benchmark_repetitions > 1).
    return {
        b["name"]: b
        for b in benchmarks
        if b.get("run_type") == "aggregate" and b.get("aggregate_name") == "mean"
    }


def main():
    parser = argparse.ArgumentParser(description=__doc__,
                                     formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--baseline",  required=True, help="Path to baseline JSON")
    parser.add_argument("--current",   required=True, help="Path to current JSON")
    parser.add_argument("--threshold", type=float, default=5.0,
                        help="Maximum allowed slowdown in percent (default: 5)")
    args = parser.parse_args()

    baseline = load_results(args.baseline)
    current  = load_results(args.current)

    if not baseline:
        print("WARNING: baseline file contains no usable results — skipping comparison.")
        sys.exit(0)

    threshold = args.threshold / 100.0
    regressions = []
    improvements = []
    compared = 0

    for name, cur in current.items():
        if name not in baseline:
            continue
        base = baseline[name]
        old_ns = base["cpu_time"]
        new_ns = cur["cpu_time"]
        if old_ns <= 0:
            continue
        compared += 1
        ratio = (new_ns - old_ns) / old_ns
        if ratio > threshold:
            regressions.append((name, old_ns, new_ns, ratio * 100))
        elif ratio < -threshold:
            improvements.append((name, old_ns, new_ns, ratio * 100))

    print(f"Compared {compared} benchmark(s) against baseline "
          f"(threshold: ±{args.threshold:.0f}%).")

    if improvements:
        print(f"\n  {len(improvements)} improvement(s):")
        for name, old, new, pct in sorted(improvements, key=lambda x: x[3]):
            print(f"    {name}")
            print(f"      {old:.2f} ns  →  {new:.2f} ns  ({pct:+.1f}%)")

    if regressions:
        print(f"\n  {len(regressions)} REGRESSION(S) above {args.threshold:.0f}% threshold:")
        for name, old, new, pct in sorted(regressions, key=lambda x: -x[3]):
            print(f"    {name}")
            print(f"      {old:.2f} ns  →  {new:.2f} ns  ({pct:+.1f}%)")
        print()
        sys.exit(1)

    print(f"  No regressions above {args.threshold:.0f}% threshold.")


if __name__ == "__main__":
    main()
