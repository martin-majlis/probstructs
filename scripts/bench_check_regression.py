#!/usr/bin/env -S uv run
# /// script
# requires-python = ">=3.8"
# dependencies = []
# ///
"""Fail with exit code 1 if benchmark results show a regression.

Two independent rules are evaluated; failing either causes a non-zero exit:

  Per-benchmark rule  — at least --min-regressions benchmarks must each be
                        slower than --threshold percent (default: 2 / 10%).
                        Guards against multiple targeted regressions.

  Geomean rule        — the geometric mean of all cpu_time ratios must not
                        exceed --geomean-threshold percent (default: 5%).
                        Guards against broad slowdowns spread across many
                        benchmarks that individually stay under the threshold.

Benchmarks present in only one file are silently skipped (new or removed).

Usage:
    uv run bench_check_regression.py \\
        --baseline old.json --current new.json \\
        [--threshold 10] [--min-regressions 2] [--geomean-threshold 5]
"""

import argparse
import json
import math
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
    parser.add_argument("--baseline",         required=True,  help="Path to baseline JSON")
    parser.add_argument("--current",          required=True,  help="Path to current JSON")
    parser.add_argument("--threshold",        type=float, default=10.0,
                        help="Per-benchmark slowdown threshold in percent (default: 10)")
    parser.add_argument("--min-regressions",  type=int,   default=2,
                        help="Minimum regressions required to fail the per-benchmark rule (default: 2)")
    parser.add_argument("--geomean-threshold", type=float, default=5.0,
                        help="Max allowed geomean slowdown in percent (default: 5)")
    args = parser.parse_args()

    baseline = load_results(args.baseline)
    current  = load_results(args.current)

    if not baseline:
        print("WARNING: baseline file contains no usable results — skipping comparison.")
        sys.exit(0)

    threshold = args.threshold / 100.0
    regressions = []
    improvements = []
    ratios = []

    for name, cur in current.items():
        if name not in baseline:
            continue
        base = baseline[name]
        old_ns = base["cpu_time"]
        new_ns = cur["cpu_time"]
        if old_ns <= 0:
            continue
        ratio = new_ns / old_ns
        ratios.append(ratio)
        delta = ratio - 1.0
        if delta > threshold:
            regressions.append((name, old_ns, new_ns, delta * 100))
        elif delta < -threshold:
            improvements.append((name, old_ns, new_ns, delta * 100))

    compared = len(ratios)
    geomean_pct = (math.exp(sum(math.log(r) for r in ratios) / compared) - 1) * 100 \
        if compared else 0.0

    print(f"Compared {compared} benchmark(s) against baseline "
          f"(threshold: ±{args.threshold:.0f}%, "
          f"min-regressions: {args.min_regressions}, "
          f"geomean-threshold: {args.geomean_threshold:.0f}%).")
    print(f"  Geomean: {geomean_pct:+.2f}%")

    if improvements:
        print(f"\n  {len(improvements)} improvement(s):")
        for name, old, new, pct in sorted(improvements, key=lambda x: x[3]):
            print(f"    {name}")
            print(f"      {old:.2f} ns  →  {new:.2f} ns  ({pct:+.1f}%)")

    if regressions:
        label = "REGRESSION(S)" if len(regressions) >= args.min_regressions else "regression(s) (below min-regressions, not failing)"
        print(f"\n  {len(regressions)} {label} above {args.threshold:.0f}% threshold:")
        for name, old, new, pct in sorted(regressions, key=lambda x: -x[3]):
            print(f"    {name}")
            print(f"      {old:.2f} ns  →  {new:.2f} ns  ({pct:+.1f}%)")

    failed = False

    if len(regressions) >= args.min_regressions:
        print(f"\nFAIL (per-benchmark): {len(regressions)} benchmark(s) regressed "
              f"above {args.threshold:.0f}% — min-regressions={args.min_regressions}.")
        failed = True

    if geomean_pct > args.geomean_threshold:
        print(f"\nFAIL (geomean): {geomean_pct:+.2f}% exceeds "
              f"geomean-threshold={args.geomean_threshold:.0f}%.")
        failed = True

    if failed:
        sys.exit(1)

    print(f"\n  OK: fewer than {args.min_regressions} per-benchmark regressions "
          f"and geomean {geomean_pct:+.2f}% within {args.geomean_threshold:.0f}% limit.")


if __name__ == "__main__":
    main()
